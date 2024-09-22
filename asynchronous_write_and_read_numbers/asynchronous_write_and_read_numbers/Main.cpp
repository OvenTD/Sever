#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include "windows.h"
#include "iostream"
#include <cstring>
#include <cstdlib>

using namespace std;

class AsyncFileProcess {
private:
    DWORD dwErrCode;
    HANDLE hSrcFile;
    OVERLAPPED wo;
    OVERLAPPED ro;
    size_t size = 512;        // ���� ũ��
    int data_count = 0;       // �Ľ̵� ���� ����

    char* writeData;          // ���Ͽ� �� ������
    char* readData;           // ���Ͽ��� ���� ������
    int* numberList;          // �Ľ̵� ���� ���

public:
    AsyncFileProcess() : dwErrCode(0) {
        wo = { 0 };
        ro = { 0 };
        readData = new char[size];  // �б� ���� �ʱ�ȭ
    }

    // ���� ���� �Լ�
    void createFile(const wchar_t* filePath) {
        hSrcFile = CreateFile(
            filePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
            FILE_FLAG_OVERLAPPED, NULL
        );
        if (hSrcFile == INVALID_HANDLE_VALUE) {
            cout << filePath << " open failed, code : " << GetLastError() << endl;
        }
    }

    // 1���� 100������ ���� ���ڿ��� ����
    char* generateNumber() {
        char* returnChar = new char[size];
        returnChar[0] = '\0';  // �� ���ڿ��� �ʱ�ȭ
        char buffer[4];

        for (int i = 1; i <= 100; ++i) {
            snprintf(buffer, sizeof(buffer), "%d ", i);
            strcat_s(returnChar, size, buffer);
        }
        strcat_s(returnChar, size, "\n");
        return returnChar;
    }

    // ���� �����͸� �Ľ��Ͽ� ���� �迭�� ��ȯ
    void parseData() {
        int count = 0;
        int* result = nullptr;    // �������� �Ҵ��� �迭�� ���� ������
        int capacity = 10;        // �ʱ� �迭 ũ��
        result = (int*)malloc(capacity * sizeof(int));

        // �Է� ���ڿ� ����
        char* dataCopy = new char[strlen(readData) + 1];
        strcpy(dataCopy, readData);

        // ������ �������� ��ūȭ
        char* token = strtok(dataCopy, " ");

        while (token != NULL) {
            if (count >= capacity) {
                capacity *= 2;  // �迭 ũ�� Ȯ��
                result = (int*)realloc(result, capacity * sizeof(int));
            }
            result[count++] = atoi(token);  // ���ڿ��� ������ ��ȯ
            token = strtok(NULL, " ");
        }

        delete[] dataCopy;  // �������� �Ҵ��� �޸� ����
        result = (int*)realloc(result, count * sizeof(int));  // �迭 ũ�� ����

        numberList = result;
        data_count = count;  // ���� ���� ����
    }

    // �迭�� �ִ� ��� ���ڸ� ���ϴ� �Լ�
    int sumArray() {
        int sum = 0;
        for (int i = 0; i < data_count; ++i) {
            sum += numberList[i];
        }
        return sum;
    }

    // ���Ͽ� �����͸� �񵿱�� ����
    void writeFile(const char* writeData) {
        DWORD bytesWritten;
        bool bIsOK = WriteFile(
            hSrcFile, writeData, strlen(writeData), &bytesWritten, &wo
        );
        if (!bIsOK) {
            dwErrCode = GetLastError();
            if (dwErrCode != ERROR_IO_PENDING) {
                printf("Write file error, code: %d\n", dwErrCode);
            }
        }
    }

    // ���Ͽ��� �����͸� �񵿱�� �б�
    void readFile() {
        BOOL bIsOK = ReadFile(
            hSrcFile, readData, size, NULL, &ro
        );
    }

    // ��ü ���� ����
    void run(const wchar_t* filePath) {
        writeData = generateNumber();     // ������ ����
        createFile(filePath);             // ���� ����
        writeFile(writeData);             // ������ ����
        DWORD dwWaitRet = WaitForSingleObject(hSrcFile, INFINITE);  // ���� �Ϸ� ���

        readFile();                       // ������ �б�
        dwWaitRet = WaitForSingleObject(hSrcFile, INFINITE);  // �б� �Ϸ� ���
        parseData();                      // ���� ������ �Ľ�
        int result = sumArray();          // ���� �ջ�

        printf("Data sum result : %d", result);  // ��� ���
    }

    // �Ҹ���
    ~AsyncFileProcess() {
        delete[] writeData;
        if (hSrcFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hSrcFile);
        }
    }
};

void _tmain(int argc, _TCHAR* argv[]) {
    if (argc < 2) {
        cout << "Usage: FileCopySync SourceFile" << endl;
        return;
    }
    AsyncFileProcess process;
    process.run(argv[1]);  // ���� ��� ����
}
