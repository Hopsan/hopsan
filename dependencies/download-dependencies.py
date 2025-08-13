#!/usr/bin/env python3

import argparse
import hashlib
import shutil
import sys
import os
import stat
import platform
import tarfile
import tempfile
import zipfile
import json
import xml.etree.ElementTree as ET
if sys.version_info.major == 2:
    import urllib
else:
    import urllib.request
if platform.system() == 'Windows':
    # Disable SSL verification on Windows, for some reason connections to Sourceforge will fail
    # The downloaded files are checksum verified so this should be safe
    import ssl
    ssl._create_default_https_context = ssl._create_unverified_context


def match_platform(platform_name):
    actual = platform.system().lower()
    return platform_name == actual


def get_attribute(element, attribute_name):
    if attribute_name in element.attrib:
        return element.attrib[attribute_name]
    return str()


def get_file_name_from_url(path):
    return os.path.basename(path)


def decide_file_name(dependency_name, url):
    fname = get_file_name_from_url(url)
    if dependency_name not in fname.lower():
        fname = dependency_name + '-' + fname
    return fname


def get_expected_hash_and_algo(releasefile_element):
    algos = ('md5', 'sha1', 'sha256')
    for algo in algos:
        if algo in releasefile_element.attrib:
            return (algo, releasefile_element.attrib[algo])
    return ('nohash_algo', 'nohash')


def hashsum_file(filepath, algorithm):
    with open(filepath, 'rb') as fileobj:
        hashobj = hashlib.new(algorithm)
        hashobj.update(fileobj.read())
        return hashobj.hexdigest()


def verify_filehash(filepath, algorithm, hashsum):
    return hashsum_file(filepath, algorithm) == hashsum.lower()


def download(url, dst, hash_algo, expected_hashsum):
    print('Info: Downloading '+url)
    try:
        if sys.version_info.major == 2:
            temp_file_name, headers = urllib.urlretrieve(url)
        else:
            temp_file = tempfile.NamedTemporaryFile(delete=False)
            with urllib.request.urlopen(url) as response:
                size = response.getheader('Content-Length', 'Unknown')
                print('Info: Size '+size+' byte')
                headers = response.getheaders()
                shutil.copyfileobj(response, temp_file)
            temp_file_name = temp_file.name
            temp_file.close()
        if verify_filehash(temp_file_name, hash_algo, expected_hashsum):
            shutil.move(temp_file_name, dst)
            os.chmod(dst, stat.S_IRUSR | stat.S_IRGRP | stat.S_IROTH | stat.S_IWUSR | stat.S_IWGRP)
            print('Info: Success downloading and verifying '+dst)
            return True
        else:
            print(headers)
            print('Error: The download failed, invalid checksum of retreived file: '+temp_file_name)
    except Exception as the_exception:
        print('Error: Could not download file, exception thrown')
        print(str(the_exception))
    return False


def clear_and_unpack(archive_file, dep_name):
    codedir = dep_name+'-code'
    builddir = dep_name+'-build'
    installdir = dep_name

    if os.path.exists(installdir):
        print('Info: Removing existing directory ' + installdir)
        shutil.rmtree(installdir)
    if os.path.exists(builddir):
        print('Info: Removing existing directory ' + builddir)
        shutil.rmtree(builddir)
    if os.path.exists(codedir):
        print('Info: Removing existing directory ' + codedir)
        shutil.rmtree(codedir)

    print('Info: Unpacking '+archive_file)
    if archive_file.endswith('.zip'):
        with zipfile.ZipFile(archive_file) as zipf:
            zipf.extractall(codedir)
    else:
        with tarfile.open(archive_file, 'r:*') as tarf:
            def is_within_directory(directory, target):
                
                abs_directory = os.path.abspath(directory)
                abs_target = os.path.abspath(target)
            
                prefix = os.path.commonprefix([abs_directory, abs_target])
                
                return prefix == abs_directory
            
            def safe_extract(tar, path=".", members=None, *, numeric_owner=False):
            
                for member in tar.getmembers():
                    member_path = os.path.join(path, member.name)
                    if not is_within_directory(path, member_path):
                        raise Exception("Attempted Path Traversal in Tar File")
            
                tar.extractall(path, members, numeric_owner=numeric_owner) 
                
            
            safe_extract(tarf, codedir)

    # Figure out the name of the unpacked root dir
    root_dir_name = str()
    for path, subdirs, files in os.walk(codedir):
        if len(subdirs) == 1 and len(files) == 0:
            root_dir_name = subdirs[0]
        break

    # Move all content from the root into codedir
    root_path = os.path.join(codedir, root_dir_name)
    for path, subdirs, files in os.walk(root_path):
        for file in files:
            shutil.move(os.path.join(root_path, file), codedir)
        for subdir in subdirs:
            shutil.move(os.path.join(root_path, subdir), codedir)
        break
    shutil.rmtree(root_path)


def join_name_version_type(name, version, dep_type=None):
    dep_item = name
    if version:
        dep_item = dep_item + ':' + version
    if dep_type:
        dep_item = dep_item + ':' + dep_type
    return dep_item


def split_name_version_type(full_name):
    name = str()
    version = str()
    dep_type = str()
    parts = full_name.split(':')
    if len(parts) >= 1:
        name = parts[0]
    if len(parts) >= 2:
        version = parts[1]
    if len(parts) >= 3:
        dep_type = parts[2]
    return (name, version, dep_type)


class DependenciesXML:
    def __init__(self, dependencies_xml_file):
        tree = ET.parse(dependencies_xml_file)
        self.root = tree.getroot()
        self.cache_dir = str()

    def __match_choice(self, name, version, choices, allow_missing):
        found_name = False
        found_version = False
        for choice in choices:
            cname, cversion, _ = split_name_version_type(choice)
            if cname == name:
                found_name = True
            if cversion and cversion == version:
                found_version = True
        if found_name and found_version:
            return True
        elif not version and found_name:
            return True
        elif allow_missing and not found_name:
            return True
        return False

    def __download_and_check_releasefile(self, dep_name, releasefile_element, force):
        hash_algo, expected_hashsum = get_expected_hash_and_algo(releasefile_element)
        for url_element in releasefile_element:
            url = url_element.text
            fname = decide_file_name(dep_name, url)

            do_download = True

            use_cache = len(self.cache_dir) > 0
            cached_fpath = os.path.join(self.cache_dir, fname)
            if use_cache and not os.path.isfile(fname) and os.path.isfile(cached_fpath):
                print("Found {} in download cache".format(fname))
                if verify_filehash(cached_fpath, hash_algo, expected_hashsum):
                    print("Copying {}".format(cached_fpath))
                    shutil.copyfile(cached_fpath, fname)
                else:
                    print('Warning: ' + hash_algo + ' missmatch in file ' + cached_fpath)
                    print('Expected: ' + expected_hashsum)
                    print('Actual: ' + hashsum_file(cached_fpath, hash_algo))

            if os.path.isfile(fname):
                print('Info: File already exists '+fname)
                if verify_filehash(fname, hash_algo, expected_hashsum):
                    print('Info: File checksum verified OK '+fname)
                    return (fname, False, True)

                print('Warning: ' + hash_algo + ' missmatch in file ' + fname)
                print('Expected: ' + expected_hashsum)
                print('Actual: ' + hashsum_file(fname, hash_algo))
                do_download = force

            if do_download:
                isok = download(url, fname, hash_algo, expected_hashsum)
                if isok:
                    if use_cache:
                        print("Copying {} to download cache {}".format(fname, cached_fpath))
                        shutil.copyfile(fname, cached_fpath)
                    return (fname, True, True)

        return ("", False, False)

    def __get_dependencies_matching_choice(self, chosen_deps, choose_all):
        allready_added_names = list()
        matching_dependencies = list()
        for dep in self.root:
            dep_name = dep.attrib['name']
            dep_version = get_attribute(dep, 'version')
            found_match = self.__match_choice(dep_name, dep_version, chosen_deps, choose_all)
            if found_match and dep_name not in allready_added_names:
                matching_dependencies.append(dep)
                allready_added_names.append(dep_name)
        return matching_dependencies

    def set_download_cache_dir(self, dir_path):
        self.cache_dir = dir_path

    def list_dependencies(self):
        names = list()
        for dep in self.root:
            dep_name = join_name_version_type(dep.attrib['name'],
                                              get_attribute(dep, 'version'),
                                              get_attribute(dep, 'type'))
            names.append(dep_name)
        return names

    def ouput_dependencies_flatpak(self):
        datas = list()
        for dep in self.root:
            dep_name = dep.attrib['name']
            dep_type = get_attribute(dep, 'type')
            dep_files = list()
            for releasefile_element in dep:
                if releasefile_element.tag == 'releasefile':
                    platform_restriction = get_attribute(releasefile_element, 'platform')
                    if platform_restriction != 'windows' and dep_type != 'toolchain':
                        depfile = dict()
                        hash_algo, expected_hashsum = get_expected_hash_and_algo(releasefile_element)
                        depfile[hash_algo] = expected_hashsum
                        depfile['url'] = list()
                        for url_element in releasefile_element:
                            url = url_element.text
                            depfile['url'].append(url)
                        url = depfile['url'][0]
                        fname = decide_file_name(dep_name, url)
                        depfile['dest-filename'] = fname
                        depfile['dest'] = 'dependencies'
                        dep_files.append(depfile)
            if len(dep_files) > 0:
                data = dict()
                data['type'] = 'file'
                dep_file = dep_files[0]  # Only one supported
                urls = dep_file['url']
                dep_file['url'] = urls[0]
                dep_file['mirror-urls'] = urls[1:]
                data.update(dep_file)
                datas.append(data)
        return datas

    def check_choices(self, choices):
        for choice in choices:
            found_names = list()
            found = False
            for dep_item in self.list_dependencies():
                dname, dversion, _ = split_name_version_type(dep_item)
                cname, cversion, _ = split_name_version_type(choice)
                if dname == cname:
                    found_names.append(join_name_version_type(dname,dversion))
                    if dversion == cversion:
                        found = True
                        break
            if not found:
                if len(found_names) > 0:
                    print('Warning: '+choice+' does not match! '+'Alternatives are: '+' '.join(found_names))
                else:
                    print('Warning: '+choice+' does not match! There are no alternatives.')

    def download_and_unpack_chosen_dependencies(self, choices, download_all, include_toolchain, force):
        dependencies_to_download = self.__get_dependencies_matching_choice(choices, download_all)
        all_verified = True
        for dep in dependencies_to_download:
            dep_name = dep.attrib['name']
            dep_type = get_attribute(dep, 'type')
            # Skip toolchin files unless they are explicitly flaged to be included (due to size)
            if dep_type == 'toolchain' and not include_toolchain:
                print('Skipping toolchain download: '+dep_name)
                continue
            print('--------------------'+dep_name+'------------------------------')
            files_matching_platform = list()
            files_without_platform = list()
            for releasefile in dep:
                if releasefile.tag == 'releasefile':
                    platform_restriction = get_attribute(releasefile, 'platform')
                    if match_platform(platform_restriction):
                        files_matching_platform.append(releasefile)
                    elif platform_restriction == "":
                        files_without_platform.append(releasefile)
            # If no file specified explicitly for this platform, use file with no platform specifier
            if len(files_matching_platform) == 0:
                files_matching_platform = files_without_platform

            # Trigger the actual download and unpacking
            this_verified = False
            for releasefile in files_matching_platform:
                file_name, did_download, verified_ok = self.__download_and_check_releasefile(dep_name, releasefile,  force)
                if dep_type != 'toolchain':
                    if did_download or file_name and not os.path.exists(dep_name+'-code') or file_name and force:
                        clear_and_unpack(file_name, dep_name)
                this_verified |= verified_ok
            all_verified &= this_verified
        return all_verified


if __name__ == "__main__":
    argparser = argparse.ArgumentParser()
    argparser.add_argument('--all',  dest='download_all', action='store_true',  help='Download all dependencies (excluding toolchain)' )
    argparser.add_argument('--cache', dest='cache_dir', type=str,  help='Cache directory for downloads')
    argparser.add_argument('--include-toolchain',  dest='download_toolchain', action='store_true',  help='Download toolchain dependencies' )
    argparser.add_argument('--list',  dest='list', action='store_true',  help='List available dependencies' )
    argparser.add_argument('--force', dest='force', action='store_true',
        help='Force download even if file exists, code, build and installation directories will be reset')
    argparser.add_argument('--output-flatpak', dest='output_flatpak', action='store_true', help='Generate for flatpak manifest')

    argparser.add_argument('dependency_name', nargs='*', action='append', type=str,
                                             help='Space separated list of dependencies to download')

    args = argparser.parse_args()
    chosen_deps = args.dependency_name[0]

    deps_xml = DependenciesXML('dependencies.xml')
    if args.cache_dir:
        print("Using download cache dir: " + args.cache_dir)
        if not os.path.isdir(args.cache_dir):
            print('Error: {} does not exist or is not a directory'.format(args.cache_dir))
            sys.exit(1)
        deps_xml.set_download_cache_dir(args.cache_dir)

    if args.output_flatpak:
        deps = deps_xml.ouput_dependencies_flatpak()
        output = str()
        for dep in deps:
            output += json.dumps(dep)+',\n'
        print(output)

    if args.list:
        names = deps_xml.list_dependencies()
        for name in names:
            print(name)
    deps_xml.check_choices(chosen_deps)
    all_ok = deps_xml.download_and_unpack_chosen_dependencies(chosen_deps, args.download_all, args.download_toolchain, args.force)
    if all_ok:
        sys.exit(0)
    else:
        sys.exit(1)
