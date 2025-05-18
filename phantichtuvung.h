#ifndef PHAN_TICH_TU_VUNG_H
#define PHAN_TICH_TU_VUNG_H

void readSourceFromFile(const char *filename, char *buffer, int maxSize);
void tokenize(char *source, const char *outputFile);

#endif