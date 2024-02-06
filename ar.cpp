#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>

const size_t SZ_HEADER = 14;
const size_t SZ_SZ_BLOCK = 4;
const size_t SZ_SZ_JSON = 5;

size_t buf_lookup_lf(const char* buf, size_t idx, size_t max) {
    while ((idx < max) && (buf[idx] != '\n') ) {
        idx = idx + 1;
    }
    return idx;
}

size_t buf_insert(char* buf, size_t idx, const char* ext, size_t sz) {
    memcpy(&buf[idx], ext, sz);
    return idx + sz;
}

void scatter(const std::filesystem::path& fn) {
    auto dn = fn;
    dn = dn.replace_extension("");
    if (std::filesystem::is_directory(dn)) {
        std::cout << "ERROR: folder" << dn << " already exists" << std::endl;
        return;
    }
    std::filesystem::create_directory(dn);

    std::ifstream fis;
    fis.open(fn, std::ios::binary | std::ios::in);
    if (!fis.is_open()) {
        std::cout << "ERROR: Can't open file " << fn << std::endl;
        return;
    }

    char header[SZ_HEADER];
    fis.read(header, SZ_HEADER);

    if (strncmp(header, "C2AR", SZ_HEADER) != 0) {
        std::cout << "ERROR: invalid file format" << std::endl;
        return;
    }

    std::ofstream  fosi;
    fosi.open(dn / "index.txt", std::ios::out);

    fosi << "header.bin" << std::endl;

    std::ofstream fos;
    fos.open(dn / "header.bin", std::ios::binary | std::ios::out);
    fos.write(header, SZ_HEADER);
    fos.close();

    int no = 1;
    while (fis.peek() != EOF) {
        size_t sz = 0;
        fis.read((char*) &sz, SZ_SZ_BLOCK);

        char buf[sz];
        fis.read(buf, sz);

        size_t idx_1 = buf_lookup_lf(buf, 0, sz) + 1;
        if (idx_1 != 1) {
            std::cout << "ERROR: Invalid file format (at " << no << " part)" << std::endl;
            break;
        }
        size_t idx_2 = buf_lookup_lf(buf, idx_1 + 1, sz) + 1;
        if (idx_2 > sz) {
            std::cout << "ERROR: Invalid file format (at " << no << " part)" << std::endl;
            break;
        }

        std::string nx = std::string(&buf[idx_1], idx_2 - idx_1 - 1)  + ".json";

        fosi << nx << std::endl;

        fos.open(dn / nx, std::ios::binary | std::ios::out);
        fos.write(&buf[idx_2], sz - idx_2);
        fos.close();

        no++;
    }

    fosi.close();
    fis.close();
}

void gather(const std::filesystem::path& dn) {
    auto fn = dn;
    fn = fn.replace_extension(".sav");
    if (std::filesystem::is_regular_file(fn)) {
        std::cout << "ERROR: file" << fn << " already exists" << std::endl;
        return;
    }

    std::ofstream fos;
    fos.open(fn,std::ios::binary | std::ios::out);

    std::ifstream fisi;
    fisi.open(dn / "index.txt", std::ios::in);

    std::ifstream fis;

    std::string l;
    while(std::getline(fisi, l)) {
        size_t sz = std::filesystem::file_size(dn / l);
        fis.open(dn / l, std::ios::binary | std::ios::in);

        if (l.ends_with(".bin")) {
            char buf[sz];

            fis.read(buf, sz);
            fis.close();

            fos.write(buf, sz);
        } else {
            size_t lz = l.size() - SZ_SZ_JSON;
            size_t xz = lz + sz + 2;

            char buf[xz];
            size_t idx = 0;
            idx = buf_insert(buf, idx, "\n", 1);
            idx = buf_insert(buf, idx, l.c_str(), lz);
            idx = buf_insert(buf, idx, "\n", 1);

            fis.read(&buf[idx], sz);
            fis.close();

            fos.write((char*) &xz, SZ_SZ_BLOCK);
            fos.write(buf, xz);
        }
    }

    fos.close();
}
