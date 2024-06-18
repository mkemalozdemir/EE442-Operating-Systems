#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <endian.h>

#define BLOCK_SIZE 512
#define FAT_SIZE 4096
#define FILE_LIST_SIZE 128  //Needed numbers are declared
#define MAX_FILE_NAME 248
#define DISK "disk.image"

typedef struct                          //One of the File List's row is declared as struct 
{
    char fileName[MAX_FILE_NAME];
    unsigned int firstBlock;
    unsigned int fileSize;
}FileListRow;

unsigned int FAT[FAT_SIZE];             //FAT is declared as unsigned int array
FileListRow fileList[FILE_LIST_SIZE];   //File List is declared as array that holds rows 
char Data[FAT_SIZE * BLOCK_SIZE];       //Total Data space is declared as char array

//Function Declarations
void initFS(void); //Initialize the file system
void Load(void);   //Loads bytes from disk to FAT, File List and Data
void Store(void);  //Stores FAT, File List and Data into Disk
unsigned int ConvertLE(unsigned int  value);
void Format(void);
void Write(char* srcPath, char* destFileName);
void Read(char* srcFileName, char *destPath);
void Delete(char* filename);
void List(void);
void Sort(void);
void Rename(char* srcFileName, char* newFileName);
void Duplicate(char* srcFileName);
void Search(char* srcFileName);
void Hide(char* srcFileName);
void Unhide(char * srcFileName);
void PrintFileList(void);
void PrintFAT(void);
void Defragment(void);


int main(int argc, char *argv[]) { //Main function calls the desired function according to the input
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <disk> <command> [args]\n", argv[0]);
        return 1;
    }

    const char *command = argv[2];

    Load(); //Loads Disk

    if (strcmp(command, "-format") == 0) 
    {
        Format();
    } 
    else if (strcmp(command, "-write") == 0) 
    {
        if (argc != 5) {
            fprintf(stderr, "Usage: %s <disk> -write <source_file> <destination_file>\n", argv[0]);
            return 1;
        }
        Write(argv[3], argv[4]);
    } 
    else if (strcmp(command, "-read") == 0) 
    {
        if (argc != 5) {
            fprintf(stderr, "Usage: %s <disk> -read <source_file> <destination_file>\n", argv[0]);
            return 1;
        }
        Read(argv[3], argv[4]);
    } 
    else if (strcmp(command, "-delete") == 0) 
    {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s <disk> -delete <file>\n", argv[0]);
            return 1;
        }
        Delete(argv[3]);
    } 
    else if (strcmp(command, "-list") == 0) 
    {
        List();
    } 
    else if (strcmp(command, "-sorta") == 0) 
    {
        Sort();
    } 
    else if (strcmp(command, "-rename") == 0) 
    {
        if (argc != 5) {
            fprintf(stderr, "Usage: %s <disk> -rename <source_file> <new_name>\n", argv[0]);
            return 1;
        }
        Rename(argv[3], argv[4]);
    } 
    else if (strcmp(command, "-duplicate") == 0) 
    {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s <disk> -duplicate <source_file>\n", argv[0]);
            return 1;
        }
        Duplicate(argv[3]);
    } 
    else if (strcmp(command, "-search") == 0) 
    {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s <disk> -search <source_file>\n", argv[0]);
            return 1;
        }
        Search(argv[3]);
    } 
    else if (strcmp(command, "-hide") == 0) 
    {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s <disk> -hide <source_file>\n", argv[0]);
            return 1;
        }
        Hide(argv[3]);
    } 
    else if (strcmp(command, "-unhide") == 0) 
    {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s <disk> -unhide <source_file>\n", argv[0]);
            return 1;
        }
        Unhide(argv[3]);
    } 
    else if (strcmp(command, "-printfilelist") == 0) 
    {
        PrintFileList();
    } 
    else if (strcmp(command, "-printfat") == 0) 
    {
        PrintFAT();
    } 
    else if (strcmp(command, "-defragment") == 0) 
    {
        Defragment();
    } 
    else 
    {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}

unsigned int ConvertLE(unsigned int  value)
{
    unsigned int  result = 0;

    result |= (value & 0x000000FF) << 24;
    result |= (value & 0x0000FF00) << 8;
    result |= (value & 0x00FF0000) >> 8;
    result |= (value & 0xFF000000) >> 24;

    return result;
}

void initFS() 
{
    // Initialize FAT
    FAT[0] = 0xFFFFFFFF;
    for (int i = 1; i < FAT_SIZE; i++) {
        FAT[i] = 0x00000000;
    }
    // Initialize File List
    for (int i = 0; i < FILE_LIST_SIZE; i++) {
        fileList[i].fileName[0] = '\0';
        fileList[i].firstBlock = 0;
        fileList[i].fileSize = 0;
    }
    // Initialize Data
    memset(Data, 0, sizeof(Data));
}

void Load(void) {
    FILE *disk = fopen(DISK, "rb");
    if (!disk) {
        perror("disk.image could not be opened!");
        return;
    }
    fread(FAT, sizeof(FAT), 1, disk);   //Reads disk to FAT
    fread(fileList, sizeof(fileList), 1, disk); //Reads disk to fileList
    fread(Data, sizeof(Data), 1, disk); //Reads disk to Data
    fclose(disk);
}

void Store(void) {
    FILE *disk = fopen(DISK, "wb");
    if (!disk) {
        perror("disk.image could not be opened!");
        return;
    }
    fwrite(FAT, sizeof(FAT), 1, disk);  //Writes FAT to disk
    fwrite(fileList, sizeof(fileList), 1, disk); //Writes fileList to disk
    fwrite(Data, sizeof(Data), 1, disk);//Writes Data to disk
    fclose(disk);
}

void Format(void) 
{
    initFS();   //Resets FAT, fileList, Data
    FILE *disk = fopen(DISK, "wb");
    if (!disk) {
        perror("disk.image could not be opened!");
        return;
    }
    Store();    
    fclose(disk);
}

void Write(char* srcPath, char* destFileName) 
{
    FILE *src = fopen(srcPath, "rb");
    if (!src) {
        perror("Source File could not be opened!");
        return;
    }

    
    int idx = -1;
    for (int i = 0; i < FILE_LIST_SIZE; i++) {
        if (fileList[i].fileName[0] == '\0') {
            idx = i;  //First available entry
            break;
        }
    }
    if (idx == -1) {
        fprintf(stderr, "File List is FULL!\n");
        fclose(src);
        return;
    }

    strcpy(fileList[idx].fileName, destFileName); //Copy the name of destination file to the fileList
    fileList[idx].fileSize = 0; 

    
    int firstBlock = -1;
    for (int i = 1; i < FAT_SIZE; i++) {
        if (FAT[i] == 0x00000000) {
            firstBlock = i; // First available block
            break;
        }
    }
    if (firstBlock == -1) {
        fprintf(stderr, "FAT is FULL!\n");
        fclose(src);
        return;
    }

    fileList[idx].firstBlock = firstBlock;    //Set the firstBlock info of the fileList

    int prevBlock = firstBlock;
    int currentBlock;
    char buffer[BLOCK_SIZE];
    int bytesRead;

    while ((bytesRead = fread(buffer, 1, BLOCK_SIZE, src)) > 0) {
        fileList[idx].fileSize += bytesRead;
        // Read bytes are written to Data
        memcpy(Data + (prevBlock * BLOCK_SIZE), buffer, bytesRead);

        if(bytesRead < BLOCK_SIZE)
        {
            break;
        }
        currentBlock = -1;
        for (int i = prevBlock+1; i < FAT_SIZE; i++) {
            if (FAT[i] == 0x00000000) {
                currentBlock = i; // Next available block
                break;
            }
        }
        if (currentBlock == -1) {
            FAT[prevBlock] = 0xFFFFFFFF;
            fprintf(stderr, "Disk is FULL!\n");
            fclose(src);
            return;
        }
        FAT[prevBlock] = currentBlock; //To find the next available block save the current block as previous block
        prevBlock = currentBlock;
    }
    FAT[prevBlock] = 0xFFFFFFFF; //Indicator for the blocks are finished for the source file

    fclose(src);
    Store();
}

void Read(char* srcFileName, char *destPath) 
{
    
    int idx = -1;
    for (int i = 0; i < FILE_LIST_SIZE; i++) {
        if (strcmp(fileList[i].fileName, srcFileName) == 0) {
            idx = i;// Index of the file in the fileList
            break;
        }
    }
    if (idx == -1) {
        fprintf(stderr, "File could not be found!\n");
        return;
    }

    FILE *dest = fopen(destPath, "wb");
    if (!dest) {
        perror("Destination File could not be opened!");
        return;
    }
    int size = fileList[idx].fileSize;
    int currentBlock = fileList[idx].firstBlock;
    char buffer[BLOCK_SIZE];
    while (currentBlock != 0xFFFFFFFF) {
        memcpy(buffer, Data + (currentBlock * BLOCK_SIZE), BLOCK_SIZE); //Copy the data from disk to buffer
        if(size >= BLOCK_SIZE)
        {
            fwrite(buffer,1,BLOCK_SIZE,dest);
            size -= BLOCK_SIZE;
        }
        else
        {
            fwrite(buffer,1,size,dest);
        }
        currentBlock = FAT[currentBlock];
    }

    fclose(dest);
}

void Delete(char *filename) 
{
    int idx = -1;
    for (int i = 0; i < FILE_LIST_SIZE; i++) {
        if (strcmp(fileList[i].fileName, filename) == 0) {
            idx = i;  //Find the file
            break;
        }
    }
    if (idx == -1) {
        fprintf(stderr, "File could not be found!\n");
        return;
    }

    int currentBlock = fileList[idx].firstBlock;

    while (currentBlock != 0xFFFFFFFF) {    //Delete from FAT
        int nextBlock = FAT[currentBlock];
        FAT[currentBlock] = 0x00000000;
        currentBlock = nextBlock;
    }
    //Delete from  fileList and Data
    fileList[idx].fileName[0] = '\0'; 
    fileList[idx].firstBlock = 0;
    fileList[idx].fileSize = 0;
    Store(); //Save to disk
}

void List(void) 
{
    for (int i = 0; i < FILE_LIST_SIZE; i++) {
        if (fileList[i].fileName[0] != '\0' && fileList[i].fileName[0] != '.') { //If the file exists and it is not hidden
            printf("%s (%u bytes)\n", fileList[i].fileName, fileList[i].fileSize);
        }
    }
}

void Sort(void) //Basic sorting algorithm for ascending order
{
    for (int i = 0; i < FILE_LIST_SIZE - 1; i++) {
        for (int j = 0; j < FILE_LIST_SIZE - i - 1; j++) {
            if (fileList[j].fileSize > fileList[j + 1].fileSize) {
                FileListRow temp = fileList[j];
                fileList[j] = fileList[j + 1];
                fileList[j + 1] = temp;
            }
        }
    }
    Store();
}

void Rename(char* srcFileName, char* newFileName) 
{
    int idx = -1;
    for (int i = 0; i < FILE_LIST_SIZE; i++) {
        if (strcmp(fileList[i].fileName, srcFileName) == 0) {
            idx = i;  //Find the file
            break;
        }
    }
    if (idx == -1) {
        fprintf(stderr, "File could not be found!\n");
        return;
    }

    strcpy(fileList[idx].fileName, newFileName);  //Copy the new name to the fileList
    Store();    //save to disk
}

void Duplicate(char* srcFileName) 
{
    int idx = -1;
    for (int i = 0; i < FILE_LIST_SIZE; i++) {
        if (strcmp(fileList[i].fileName, srcFileName) == 0) {
            idx = i; //Find the file
            break;
        }
    }
    if (idx == -1) {
        fprintf(stderr, "File could not be found!\n");
        return;
    }

    char name[MAX_FILE_NAME];
    size_t sourceFileNameLength = strlen(fileList[idx].fileName);
    char *copy = "_copy";

    if (sourceFileNameLength + strlen(copy) < MAX_FILE_NAME) { //Cannot exceed the MAX_FILE_NAME
        snprintf(name, MAX_FILE_NAME, "%s%s", fileList[idx].fileName, copy); //name and _copy are integrated
        Write(fileList[idx].fileName, name);  //Write the duplicated file to the disk
    } else {
        fprintf(stderr, "Length of the name is too long to duplicate!\n");
    }
}

void Search(char* srcFileName) 
{
    int found = 0;
    for (int i = 0; i < FILE_LIST_SIZE; i++) {
        if (strstr(fileList[i].fileName, srcFileName) != NULL) {
            printf("YES");  //Search the file if found print YES
            found = 1;
        }
    }
    if (!found) {
        printf("NO");   //If not found print NO
    }
}

void Hide(char* srcFileName) 
{
    for (int i = 0; i < FILE_LIST_SIZE; i++) {
        if (strcmp(fileList[i].fileName, srcFileName) == 0) {
            if (fileList[i].fileName[0] != '.') { //If it is not hidden
                memmove(fileList[i].fileName + 1, fileList[i].fileName, strlen(fileList[i].fileName) + 1); //Rearrange the name space
                fileList[i].fileName[0] = '.';  //Add "."
            }
            Store();
            return;
        }
    }
    fprintf(stderr, "File could not be found!\n");
}

void Unhide(char* srcFileName) 
{
    for (int i = 0; i < FILE_LIST_SIZE; i++) {
        if (strcmp(fileList[i].fileName + 1, srcFileName) == 0) {
            if (fileList[i].fileName[0] == '.') { //If the file is hidden
                memmove(fileList[i].fileName, fileList[i].fileName + 1, strlen(fileList[i].fileName)); //Destroy the "."
            }
            Store();
            return;
        }
    }
    fprintf(stderr, "File could not be found!\n");
}

void PrintFileList(void) 
{
    FILE *fileListFile = fopen("filelist.txt", "w"); //Create text file to show file list
    if (!fileListFile) {
        perror("filelist.txt could not be opened!");
        return;
    }

    for (int i = 0; i < FILE_LIST_SIZE; i++) {
        if (fileList[i].fileName[0] != '\0') { //Print all the file list rows that exist
            fprintf(fileListFile, "%s (%u bytes) - First Block: %u\n", fileList[i].fileName, fileList[i].fileSize, fileList[i].firstBlock);
        }
    }

    fclose(fileListFile);
}

void PrintFAT(void) 
{
    unsigned int fat;
    FILE *fatFile = fopen("fat.txt", "w");
    if (!fatFile) {
        perror("fat.txt could not be opened!");
        return;
    }

    for (int i = 0; i < FAT_SIZE; i++) {    
        fat = ConvertLE(FAT[i]);
        fprintf(fatFile, "%d: %08X\n",i,fat);    //Print FAT array in hexadecimal form
    }

    fclose(fatFile);
}

void Defragment(void) 
{
    unsigned int holderFAT[FAT_SIZE]; //Holder FAT array
    memset(holderFAT, 0, sizeof(holderFAT));  //Initialize the holder FAT
    holderFAT[0] = 0xFFFFFFFF;

    char holderData[FAT_SIZE * BLOCK_SIZE];   //Holder Data array
    memset(holderData, 0, sizeof(holderData));   //Initialize the Data array

    unsigned int currentBlock = 1;
    for (int i = 0; i < FILE_LIST_SIZE; i++) {
        if (fileList[i].fileName[0] != '\0') {
            int originalBlock = fileList[i].firstBlock; //Save the original position of the file
            fileList[i].firstBlock = currentBlock;
            while (originalBlock != 0xFFFFFFFF) {
                memcpy(&holderData[currentBlock * BLOCK_SIZE], &Data[originalBlock * BLOCK_SIZE], BLOCK_SIZE); //Copy the data from disk to holder data array
                originalBlock = FAT[originalBlock]; //Take the next block of the file
                holderFAT[currentBlock] = currentBlock + 1; //Update the next block information
                currentBlock++;
            }
            holderFAT[currentBlock - 1] = 0xFFFFFFFF;  //Finalize the file in FAT
        }
    }

    memcpy(FAT, holderFAT, sizeof(holderFAT));    //copy the holder FAT to real FAT
    memcpy(Data, holderData, sizeof(holderData)); //copy the holder Data to real Data

    Store(); //save to disk
}
