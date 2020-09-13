//思考10G的数据用1G的数据进行排序问题 https://www.cnblogs.com/tangxin-blog/p/6814421.html
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

enum
{
    enmMaxFileNameLen = 256,
};

void SaveArrToFile(int32_t arrInt[], int32_t arrSize, const char *fileName);
void ReadArrFromFile(int32_t arrInt[], int32_t &arrSize, const int32_t amxArrSize, const char *fileName);
void ReadArrFromFilePtr(int32_t arrInt[], int32_t &arrSize, const int32_t amxArrSize, FILE *fp);
void RandomGenArrInt(int32_t arrInt[], int32_t arrSize);
void RandomGenData(int32_t numberCount);
void QSort(int32_t arrInt[], int32_t start, int32_t end);
void BigDataSort(const char *fileName, const int32_t maxNumberInMem);
int32_t Segment(const char *fileName, const int32_t maxNumberInMem);
int32_t MergeTwoFile(const char *fileName1, const char *fileName2, const char *fileOut);
void PrintArrInt(int32_t arr[], int32_t arrSize);

int32_t main()
{
    RandomGenData(10000);
    BigDataSort("data_10000.txt", 100);
    getchar();
    return 0;
}

void RandomGenArrInt(int32_t arrInt[], int32_t arrSize)
{
    for (int32_t i = 0; i < arrSize; ++i)
    {
        arrInt[i] = i + 1;
    }
    for (int32_t i = 1; i <= arrSize; ++i)
    {
        int32_t m = rand() % arrSize;
        int32_t n = rand() % arrSize;
        int32_t tmp = arrInt[m];
        arrInt[m] = arrInt[n];
        arrInt[n] = tmp;
    }
}

void SaveArrToFile(int32_t arrInt[], int32_t arrSize, const char *fileName)
{
    FILE *fp = NULL;
    fopen_s(&fp, fileName, "w");
    if (!fp)
    {
        printf("open %s failed!\n", fileName);
        return;
    }
    for (int32_t i = 0; i < arrSize; ++i)
    {
        fprintf_s(fp, "%d,", arrInt[i]);
    }
    fclose(fp);
    printf("save %s \n", fileName);
}

void RandomGenData(int32_t numberCount)
{
    int32_t *arr = new int32_t[numberCount];
    RandomGenArrInt(arr, numberCount);
    char fileName[enmMaxFileNameLen] = { 0 };
    sprintf_s(fileName, enmMaxFileNameLen,"data_%d.txt", numberCount);
    SaveArrToFile(arr, numberCount, fileName);
}

void QSort(int32_t arrInt[], int32_t start, int32_t end)
{
    if (start >= end){ return; }
    int32_t i = start, j = end;
    int32_t tmp = arrInt[i];
    while (i < j)
    {
        while (i < j && tmp < arrInt[j])
        {
            --j;
        }
        arrInt[i] = arrInt[j];
        while (i < j && tmp >= arrInt[i])
        {
            ++i;
        }
        arrInt[j] = arrInt[i];
    }
    arrInt[i] = tmp;
    QSort(arrInt, start, i - 1);
    QSort(arrInt, i + 1, end);
}

void ReadArrFromFile(int32_t arrInt[], int32_t &arrSize, const int32_t amxArrSize, const char *fileName)
{
    arrSize = 0;
    FILE *fp = NULL;
    fopen_s(&fp, fileName, "r");
    if (!fp)
    {
        printf("open %s failed!\n", fileName);
        return;
    }
    while (arrSize < amxArrSize && !feof(fp))
    {
        fscanf_s(fp, "%d,", &arrInt[arrSize++]);
    }
}

void ReadArrFromFilePtr(int32_t arrInt[], int32_t &arrSize, const int32_t amxArrSize, FILE *fp)
{
    arrSize = 0;
    while (arrSize < amxArrSize && !feof(fp))
    {
        fscanf_s(fp, "%d,", &arrInt[arrSize]);
        if (!feof(fp))
        {
            ++arrSize;
        }
    }
}

void BigDataSort(const char *fileName, const int32_t maxNumberInMem)
{
    int32_t segFileCount = Segment(fileName, maxNumberInMem);
    int32_t fileIndex = 1;
    char fileName1[enmMaxFileNameLen] = { 0 };
    char fileName2[enmMaxFileNameLen] = { 0 };
    char fileOut[enmMaxFileNameLen] = { 0 };
    while (true)
    {
        sprintf_s(fileName1, "%d.txt", fileIndex++);
        sprintf_s(fileName2, "%d.txt", fileIndex++);
        sprintf_s(fileOut, "%d.txt", segFileCount++);
        int32_t ret = MergeTwoFile(fileName1, fileName2, fileOut);
        if (ret != 0)
        {
            break;
        }
    }
}

int32_t Segment(const char *fileName, const int32_t maxNumberInMem)
{
    int32_t *arr = new int32_t[maxNumberInMem];
    FILE *fp = NULL;
    fopen_s(&fp, fileName, "r");
    if (!fp)
    {
        printf("open %s failed!\n", fileName);
        return 0;
    }
    int32_t tmpFileIndex = 1;
    while (true)
    {
        int32_t arrSize = 0;
        ReadArrFromFilePtr(arr, arrSize, maxNumberInMem, fp);
        if (arrSize == 0)
        {
            break;
        }
        QSort(arr, 0, arrSize - 1);
        char tmpFileName[enmMaxFileNameLen] = { 0 };
        sprintf_s(tmpFileName, enmMaxFileNameLen, "%d.txt", tmpFileIndex++);
        SaveArrToFile(arr, arrSize, tmpFileName);
    }
    fclose(fp);
    delete[] arr;
    return tmpFileIndex;
}

int32_t MergeTwoFile(const char *fileName1, const char *fileName2, const char *fileOut)
{
    int32_t ret = 1;
    FILE *fp1 = NULL, *fp2 = NULL, *fpOut = NULL;
    fopen_s(&fp1, fileName1, "r");
    fopen_s(&fp2, fileName2, "r");
    fopen_s(&fpOut, fileOut, "w");
    if (!fileOut)
    {
        printf("open %s failed!\n", fileOut);
        return ret;
    }
    int32_t val1 = 0, val2 = 0;
    if (fp1){ fscanf_s(fp1, "%d,", &val1); }
    if (fp2){ fscanf_s(fp2, "%d,", &val2); }
    while (fp1 && fp2 && !feof(fp1) && !feof(fp2))
    {
        if (val1 < val2)
        {
            // printf("%d ", val1);
            fprintf_s(fpOut, "%d,", val1);
            fscanf_s(fp1, "%d,", &val1);
        }
        else
        {
            // printf("%d ", val2);
            fprintf_s(fpOut, "%d,", val2);
            fscanf_s(fp2, "%d,", &val2);
        }
        ret = 0;
    }
    while (fp1 && !feof(fp1))
    {
        // printf("%d ", val1);
        fprintf_s(fpOut, "%d,", val1);
        fscanf_s(fp1, "%d,", &val1);
    }
    while (fp2 && !feof(fp2))
    {
        // printf("%d ", val2);
        fprintf_s(fpOut, "%d,", val2);
        fscanf_s(fp2, "%d,", &val2);
    }
    if (fp1){ fclose(fp1); }
    if (fp2){ fclose(fp2); }
    fclose(fpOut);
    printf("save %s \n", fileOut);
    return ret;
}

void PrintArrInt(int32_t arr[], int32_t arrSize)
{
    for (int32_t i = 0; i < arrSize; ++i)
    {
        printf("%d ", arr[i]);
    }
}
