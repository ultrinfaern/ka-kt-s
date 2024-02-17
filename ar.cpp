#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>

typedef uint32_t SZ_T;
typedef std::vector<char> BUF_T;

const SZ_T SZ_HEADER = 14;
const SZ_T SZ_SZ_BLOCK = 4;

SZ_T buf_lookup_lf(BUF_T& buf, SZ_T idx, SZ_T max) {
    while ((idx < max) && (buf[idx] != '\n') ) {
        idx = idx + 1;
    }
    return idx;
}

SZ_T buf_insert(BUF_T& buf, SZ_T idx, const std::string& ext) {
    SZ_T sz = ext.size();
    memcpy(&buf[idx], ext.data(), sz);
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
    fis.open(fn, std::ios::binary);
    if (!fis.is_open()) {
        std::cout << "ERROR: Can't open file " << fn << std::endl;
        return;
    }

    BUF_T buf_header;
    buf_header.resize(SZ_HEADER);
    fis.read(&buf_header[0], SZ_HEADER);

    if (strncmp(&buf_header[0], "C2AR", SZ_HEADER) != 0) {
        std::cout << "ERROR: invalid file format" << std::endl;
        return;
    }

    std::ofstream fosi;
    fosi.open(dn / "index.txt");

    fosi << "header.bin" << std::endl;

    std::ofstream fos;
    fos.open(dn / "header.bin", std::ios::binary);
    fos.write(&buf_header[0], SZ_HEADER);
    fos.close();

    int no = 1;
    while (fis.peek() != EOF) {
        SZ_T sz = 0;
        fis.read((char *) &sz, SZ_SZ_BLOCK);

        BUF_T buf;
        buf.resize(sz);
        fis.read(&buf[0], sz);

        SZ_T idx_1 = buf_lookup_lf(buf, 0, sz) + 1;
        if (idx_1 != 1) {
            std::cout << "ERROR: Invalid file format (at " << no << " part)" << std::endl;
            break;
        }
        SZ_T idx_2 = buf_lookup_lf(buf, idx_1 + 1, sz) + 1;
        if (idx_2 > sz) {
            std::cout << "ERROR: Invalid file format (at " << no << " part)" << std::endl;
            break;
        }

        std::string nx = std::string(&buf[idx_1], idx_2 - idx_1 - 1);

        fosi << nx << std::endl;

        fos.open(dn / nx, std::ios::binary);
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
    fos.open(fn,std::ios::binary);

    std::ifstream fisi;
    fisi.open(dn / "index.txt");

    std::ifstream fis;

    std::string l;
    while(std::getline(fisi, l)) {
        SZ_T sz = std::filesystem::file_size(dn / l);
        fis.open(dn / l, std::ios::binary);

        if (l.ends_with(".bin")) {
            BUF_T buf;
            buf.resize(sz);

            fis.read(&buf[0], sz);
            fis.close();

            fos.write(&buf[0], sz);
        } else {
            sz = sz + l.size() + 2;

            BUF_T buf;
            buf.resize(sz);

            SZ_T idx = 0;
            idx = buf_insert(buf, idx, "\n");
            idx = buf_insert(buf, idx, l);
            idx = buf_insert(buf, idx, "\n");

            fis.read(&buf[idx], sz);
            fis.close();

            fos.write((char*) &sz, SZ_SZ_BLOCK);
            fos.write(&buf[0], sz);
        }
    }

    fos.close();
}
