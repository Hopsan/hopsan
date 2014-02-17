    shdmem_<<<name>>> = boost::interprocess::shared_memory_object(boost::interprocess::open_or_create, "hopsan_<<<name>>>", boost::interprocess::read_write); 
    shdmem_<<<name>>>.truncate(sizeof(<<<type>>>)); 
    region_<<<name>>> = boost::interprocess::mapped_region(shdmem_<<<name>>>, boost::interprocess::read_write); 
    <<<name>>>_socket = static_cast<<<<type>>>*>(region_<<<name>>>.get_address());
