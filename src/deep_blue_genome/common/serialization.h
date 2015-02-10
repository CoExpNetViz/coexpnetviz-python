// Author: Tim Diels <timdiels.m@gmail.com>

#pragma once

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/shared_ptr.hpp>

namespace DEEP_BLUE_GENOME {

template <class T>
void load_from_binary(std::string path, T& object) {
        using namespace std;
        using namespace boost::iostreams;
        stream_buffer<file_source> buffer(path, ios::binary); // Note: this turned out to be strangely slightly faster than mapped_file
        boost::archive::binary_iarchive ar(buffer);
        ar >> object;
}

template <class T>
void save_to_binary(std::string path, const T& object) {
        using namespace std;
        using namespace boost::iostreams;
        using namespace boost::filesystem;
        create_directories(boost::filesystem::path(path).remove_filename());
        stream_buffer<file_sink> out(path, ios::binary);
        boost::archive::binary_oarchive ar(out);
        try {
                ar << object;
        }
        catch (const exception& e) {
                throw runtime_error("Error while writing to " + path + ": " + exception_what(e));
        }
}


} // end namespace



