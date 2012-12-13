    boost::interprocess::shared_memory_object shdmem_<<<name>>>(boost::interprocess::open_or_create, "hopsan_<<<name>>>", boost::interprocess::read_write); 
    shdmem_<<<name>>>.truncate(64); 
    boost::interprocess::mapped_region region_<<<name>>>(shdmem_<<<name>>>, boost::interprocess::read_write); 
    <<<name>>>_socket = static_cast<<<<type>>>*>(region_<<<name>>>.get_address());
