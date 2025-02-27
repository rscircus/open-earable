#include "FileWriter.h"

FileWriter::FileWriter() {
}

FileWriter::~FileWriter() {
}

bool FileWriter::begin() {
    return sd_manager.begin();
}

void FileWriter::end() {
    sd_manager.closeFile(&file);
    sd_manager.end();
}

void FileWriter::cleanFile() {
    sd_manager.remove(_name);
}

void FileWriter::setName(String name) {
    sd_manager.closeFile(&file);
    _name = std::move(name);
}
void FileWriter::setWriting(bool writing) {
    _writing = writing;
}

unsigned int FileWriter::write_block_at(unsigned int offset, uint8_t *block, int size) {
    if (!_writing) return 0;
    return sd_manager.write_block_at(&file, offset, block, size);
}

unsigned int FileWriter::write_block(uint8_t *block, int size) {
    if (!_writing) return 0;
    return sd_manager.write_block(&file, block, size);
}

unsigned int FileWriter::read_block_at(unsigned int offset, uint8_t *block, int size) {
    return sd_manager.read_block_at(&file, offset, block, size);
}

unsigned int FileWriter::read_block(uint8_t *block, int size) {
    return sd_manager.read_block(&file, block, size);
}

bool FileWriter::openFile() {
    file = sd_manager.openFile(_name, _writing);
    return file;
}

void FileWriter::closeFile() {
    sd_manager.closeFile(&file);
}

bool FileWriter::isOpen() {
    return file.isOpen();
}

unsigned int FileWriter::get_size() {
    return file.fileSize();
}
