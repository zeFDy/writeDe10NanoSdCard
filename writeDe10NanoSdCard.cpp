//
// Attention : 
// Lancer l'application (ou Visual Studio) en administrateur
// sinon pas possible de lire et/ou écrire sur la sdCard !
//


#include <iostream>
#include "disk.h"

FILE*       myLogFile               = 0;
#define     MAXIMUM_DUMP_SIZE       0x1000
#define     PADDED_SIZE	            0x10000
uint32_t    uiAlteraPartition       = 0;
uint32_t    uiLinuxPartition        = 0;
char*       dataToWrite             = 0;

#pragma pack(1)
typedef struct  
{
    uint8_t     ucBootIndicator;
    uint8_t     ucStartingCHSValues[3];
    uint8_t     ucPartitionTypeDescriptor;
    uint8_t     ucEndingCHSValues[3];
    uint32_t    uiStartingSector;
    uint32_t    uiPartitionSize;
}   MbrPartitionInfo;

#pragma pack(1)
typedef struct 
{
    uint8_t             ucRoutine[0x1B8];
    uint32_t            uiSignature;
    uint16_t            usNull;
    MbrPartitionInfo    InfoPart[4];
    uint16_t            usMagic;
}   MbrInfo;

void    HexDump(unsigned char* ucData, unsigned int iSize)
{
        int iDumpSize = iSize;

        if (iDumpSize > MAXIMUM_DUMP_SIZE)
        {
            iDumpSize = MAXIMUM_DUMP_SIZE;
            fprintf(myLogFile, "Dump limited to 0x%X bytes\n", iDumpSize);
        }

        char caAsciiDisplay[80];
        memset(caAsciiDisplay, 0, 80);

        fprintf(myLogFile, "0x%08X - ", 0);

        for (int    i = 0;
                    i < iDumpSize;
                    i++)
        {
            if (i != 0 && i % 16 == 0)
            {
                //printf(" - %s\n", caAsciiDisplay);
                fprintf(myLogFile, " - %s\n", caAsciiDisplay);
                fprintf(myLogFile, "0x%08X - ", (unsigned int)i);
                sprintf(caAsciiDisplay, "");
            }

            //printf("%.02X ", (unsigned char)ucData[i]);
            fprintf(myLogFile, "%.02X ", (unsigned char)ucData[i]);

            char c = (unsigned char)ucData[i];
            //if(     (c>='0' && c<='9')
            //    ||  (c>='a' && c<='z')
            //    ||  (c>='A' && c<='Z')
            //    )
            if (c >= ' ' && c <= 127)
            {
                char caDummy[2];

                caDummy[0] = c;
                caDummy[1] = 0x00;

                strcat(caAsciiDisplay, caDummy);
            }
            else
            {
                strcat(caAsciiDisplay, ".");
            }
        }
}

void    DumpPartitionTableEntry(unsigned char* ucStart, uint8_t uiPartitionIndex)
{
        unsigned int uiMyInt = 0;

        printf("[00] %.02X Boot Indicator -> ", (unsigned char*)ucStart[0]);
        fprintf(myLogFile, "[00] %.02X Boot Indicator -> ", (unsigned char*)ucStart[0]);

        
        if (ucStart[0] == 0x80)     {printf("Active\n");    fprintf(myLogFile, "Active\n");     }
        else                        {printf("Inactive\n");  fprintf(myLogFile, "Inactive\n");   }

        printf("[04] %.02X Partition Type -> ", (unsigned char*)ucStart[4]);
        fprintf(myLogFile, "[04] %.02X Partition Type -> ", (unsigned char*)ucStart[4]);
        
        switch(ucStart[4])
        {
            case    0x00:       printf("Empty partition entry");
                                fprintf(myLogFile, "Empty partition entry");
                                break;
            case    0x0C:       printf("FAT32 with LBA");
                                fprintf(myLogFile, "FAT32 with LBA");
                                break;

            case    0x83:       printf("Linux");
                                fprintf(myLogFile, "Linux");
                                uiLinuxPartition = uiPartitionIndex;
                                break;
            
            case    0xA2:       printf("Altera Cyclone V");
                                fprintf(myLogFile, "Altera Cyclone V");
                                uiAlteraPartition = uiPartitionIndex;
                                break;
            
            default:            printf("I don't know..");
                                fprintf(myLogFile, "I don't know..");
        }

        printf("\n");
        fprintf(myLogFile, "\n");

        printf("[01] %.02X%.02X%.02X   Starting CHS Value\n", (unsigned char*)ucStart[1], (unsigned char*)ucStart[2],(unsigned char*)ucStart[3] );
        fprintf(myLogFile, "[01] %.02X%.02X%.02X   Starting CHS Value\n", (unsigned char*)ucStart[1], (unsigned char*)ucStart[2], (unsigned char*)ucStart[3]);
        uiMyInt = ucStart[1] + 256 * ucStart[2] + 65536 * ucStart[3];
        printf("     %.06X\n", uiMyInt);
        fprintf(myLogFile, "     %.06X\n", uiMyInt);

        printf("[05] %.02X%.02X%.02X   Ending CHS Value\n", (unsigned char*)ucStart[5], (unsigned char*)ucStart[6],(unsigned char*)ucStart[7] );
        fprintf(myLogFile, "[05] %.02X%.02X%.02X   Ending CHS Value\n", (unsigned char*)ucStart[5], (unsigned char*)ucStart[6], (unsigned char*)ucStart[7]);
        uiMyInt = ucStart[5] + 256 * ucStart[6] + 65536 * ucStart[7];
        printf("     %.06X\n", uiMyInt);
        fprintf(myLogFile, "     %.06X\n", uiMyInt);

        printf("[08] %.02X%.02X%.02X%.02X Starting Sector\n", (unsigned char*)ucStart[8], (unsigned char*)ucStart[9],(unsigned char*)ucStart[10],(unsigned char*)ucStart[11] );
        fprintf(myLogFile, "[08] %.02X%.02X%.02X%.02X Starting Sector\n", (unsigned char*)ucStart[8], (unsigned char*)ucStart[9], (unsigned char*)ucStart[10], (unsigned char*)ucStart[11]);
        uiMyInt = ucStart[8] + 256 * ucStart[9] + 65536 * ucStart[10] + 16777216 * ucStart[11];
        printf("     %.08X\n", uiMyInt);
        fprintf(myLogFile, "     %.08X\n", uiMyInt);

        printf("[12] %.02X%.02X%.02X%.02X Partition Size (in sectors)\n", (unsigned char*)ucStart[12], (unsigned char*)ucStart[13],(unsigned char*)ucStart[14],(unsigned char*)ucStart[15] );
        fprintf(myLogFile, "[12] %.02X%.02X%.02X%.02X Partition Size (in sectors)\n", (unsigned char*)ucStart[12], (unsigned char*)ucStart[13], (unsigned char*)ucStart[14], (unsigned char*)ucStart[15]);
        uiMyInt = ucStart[12] + 256 * ucStart[13] + 65536 * ucStart[14] + 16777216 * ucStart[15];
        printf("     %.08X\n", uiMyInt);
        fprintf(myLogFile, "     %.08X\n", uiMyInt);


        
}

int     main(int argc, char* argv[])
{
        #define DEFAULT_INPUT_FILENAME		    "myBoot.img"
        #define DEFAULT_DRIVE_LETTER            'J'

        #define OFFSET_INFO_PARTITION1_IN_MBR   0x1BE
        #define OFFSET_INFO_PARTITION2_IN_MBR   0x1CE
        #define OFFSET_INFO_PARTITION3_IN_MBR   0x1DE
        #define OFFSET_INFO_PARTITION4_IN_MBR   0x1EE
        #define OFFSET_INFO_MAGIC_IN_MBR        0x1FE


        //char   cDriveLetter = 'J';                 // J sur PC i9
        //char   cDriveLetter = 'E';                 // E sur PC i5 Laptop Boulot

        char        cDriveLetter         = DEFAULT_DRIVE_LETTER;
        char        caInputFileName[256] = DEFAULT_INPUT_FILENAME;
        uint32_t    uiSizeOfMbr = sizeof(MbrInfo);

       if (argc >= 2)       // 1er argument (optionnel) est la lettre 
       {
           std::cout << "argv[1]  = " << argv[1] << "\n";
           cDriveLetter = argv[1][0];
       }

       if (argc >= 3)       // 2eme argument (optionnel) est le fichier 
       {
           std::cout << "argv[2]  = " << argv[2] << "\n";
           memset(caInputFileName, 0, 256);
           if (strlen(argv[2]) < 256)       strncpy(caInputFileName, argv[2], strlen(argv[2]));
           else                             exit(-1);
       }

       myLogFile = fopen("logFile.txt", "wt");

       std::cout << "----------------------------\n";
       std::cout << "  Run this tool as admin !\n";
       std::cout << "----------------------------\n";
       int volumeID = cDriveLetter - 'A';
   
       HANDLE               hFile;
       HANDLE               hReadRawDisk;
       unsigned long long   sectorsize;
       char                 *sectorData;
       unsigned long long   i, numsectors;
       unsigned int         uiMaxSectorLoadingSize;

       // acces en lecture
       HANDLE hVolume	= getHandleOnVolume(volumeID, GENERIC_READ);
       DWORD deviceID	= getDeviceID(hVolume);
   
       if (!getLockOnVolume(hVolume))
       {
           CloseHandle(hVolume);
           exit(-1);
       }

       if (!unmountVolume(hVolume))
       {
           removeLockOnVolume(hVolume);
           CloseHandle(hVolume);
           exit(-1);
       }

       hReadRawDisk = getHandleOnDevice(deviceID, GENERIC_READ);
       if (hReadRawDisk == INVALID_HANDLE_VALUE)
       {
           removeLockOnVolume(hVolume);
           //CloseHandle(hFile);
           CloseHandle(hVolume);
           exit(-1);
       }

       numsectors = getNumberOfSectors(hReadRawDisk, &sectorsize);
       printf("numsectors = %d\n", numsectors);
       fprintf(myLogFile, "numsectors = %d\n", numsectors);

       uiMaxSectorLoadingSize = MAXIMUM_DUMP_SIZE / sectorsize;


        // Read MBR partition table
        printf("Read MBR partition table\n");
        fprintf(myLogFile, "Read MBR partition table\n");
        sectorData = readSectorDataFromHandle(hReadRawDisk, 0, 1ul, 512ul);
        HexDump((unsigned char*)sectorData, 512);
        printf("\n");
        fprintf(myLogFile, "\n");

        printf("MAGIC NUMBER : %.02X %.02X\n", (unsigned char) sectorData[0x01FE], (unsigned char) sectorData[0x01FF]);
        fprintf(myLogFile, "MAGIC NUMBER : %.02X %.02X\n", (unsigned char)sectorData[0x01FE], (unsigned char)sectorData[0x01FF]);

        printf("\n");
        fprintf(myLogFile, "\n");
        printf("Read MBR 1st partition :\n");
        fprintf(myLogFile, "Read MBR 1st partition :\n");
        HexDump((unsigned char*)&sectorData[OFFSET_INFO_PARTITION1_IN_MBR], 16);
        printf("\n");
        fprintf(myLogFile, "\n");
        DumpPartitionTableEntry((unsigned char*)&sectorData[OFFSET_INFO_PARTITION1_IN_MBR], 0);

        printf("\n");
        fprintf(myLogFile, "\n");
        printf("Read MBR 2nd partition :\n");
        fprintf(myLogFile, "Read MBR 2nd partition :\n");
        HexDump((unsigned char*)&sectorData[OFFSET_INFO_PARTITION2_IN_MBR], 16);
        printf("\n");
        fprintf(myLogFile, "\n");
        DumpPartitionTableEntry((unsigned char*)&sectorData[OFFSET_INFO_PARTITION2_IN_MBR], 1);

        printf("\n");
        fprintf(myLogFile, "\n");
        printf("Read MBR 3rd partition :\n");
        fprintf(myLogFile, "Read MBR 3rd partition :\n");
        HexDump((unsigned char*)&sectorData[OFFSET_INFO_PARTITION3_IN_MBR], 16);
        printf("\n");
        fprintf(myLogFile, "\n");
        DumpPartitionTableEntry((unsigned char*)&sectorData[OFFSET_INFO_PARTITION3_IN_MBR], 2);

        printf("\n");
        fprintf(myLogFile, "\n");
        printf("Read MBR 4th partition :\n");
        fprintf(myLogFile, "Read MBR 4th partition :\n");
        HexDump((unsigned char*)&sectorData[OFFSET_INFO_PARTITION4_IN_MBR], 16);
        printf("\n");
        fprintf(myLogFile, "\n");
        DumpPartitionTableEntry((unsigned char*)&sectorData[OFFSET_INFO_PARTITION4_IN_MBR], 3);
        
//        numsectors = 1ul;
//        // Read partition information
//        for (i=0ul; i<4ul; i++)
//        {
//            uint32_t partitionStartSector = *((uint32_t*) (sectorData   + OFFSET_INFO_PARTITION1_IN_MBR + 8     + 16*i));
//            uint32_t partitionNumSectors = *((uint32_t*) (sectorData    + OFFSET_INFO_PARTITION1_IN_MBR + 12    + 16*i));
//        
//            fprintf(myLogFile, "\n");
//            fprintf(myLogFile, "[%d] partitionStartSector : 0x%.08X (%d)\n", i, partitionStartSector,    partitionStartSector);
//            fprintf(myLogFile, "[%d] partitionNumSectors  : 0x%.08X (%d)\n", i, partitionNumSectors,     partitionNumSectors);
//
//            // Set numsectors to end of last partition
////            if (partitionStartSector + partitionNumSectors > numsectors)
////            {
////                numsectors = partitionStartSector + partitionNumSectors;
////            }
//
//            // 3rd Partition is Cyclone V :
//            // read & dump the data of this partition
//            // start sector = 0x0800 - partition size = 0x1000
//
//            int iNumOfSectorToLoad = partitionNumSectors;
//            if (iNumOfSectorToLoad > uiMaxSectorLoadingSize)   iNumOfSectorToLoad = uiMaxSectorLoadingSize;
//
//            char* partitionData = readSectorDataFromHandle(hReadRawDisk, partitionStartSector, iNumOfSectorToLoad /*partitionNumSectors*/, 512ul);
//
//            printf("Dump partition %d :\n", i+1);
//            fprintf(myLogFile, "Dump partition %d :\n", i + 1);
//            HexDump((unsigned char*)partitionData, iNumOfSectorToLoad * 512);
//
//        }

        //// copy altera partition to linux partition (to try write Sector)
        //uint32_t alteraPartitionStartSector   = *((uint32_t*)(sectorData + OFFSET_INFO_PARTITION1_IN_MBR + 8  + 16 * uiAlteraPartition));
        //uint32_t alteraPartitionNumSectors    = *((uint32_t*)(sectorData + OFFSET_INFO_PARTITION1_IN_MBR + 12 + 16 * uiAlteraPartition));

        ////// Set numsectors to end of last partition
        ////if (partitionStartSector + partitionNumSectors > numsectors)
        ////{
        ////    numsectors = partitionStartSector + partitionNumSectors;
        ////}
        //        
        //int iNumOfSectorToLoad = alteraPartitionNumSectors;
        //if (iNumOfSectorToLoad > uiMaxSectorLoadingSize)   iNumOfSectorToLoad = uiMaxSectorLoadingSize;

        //char* dataToWrite = readSectorDataFromHandle(hReadRawDisk, alteraPartitionStartSector, 8 /*iNumOfSectorToLoad*/, 512ul);

        ////printf("\n\n\nDump partition %d :\n", uiAlteraPartition + 1);
        ////fprintf(myLogFile, "\n\n\nDump partition %d :\n", uiAlteraPartition + 1);
        ////HexDump((unsigned char*)partitionData, partitionNumSectors * 512);

        removeLockOnVolume(hVolume);
        CloseHandle(hReadRawDisk);
        CloseHandle(hVolume);

        // lecture du fichier
        FILE* myFile = NULL;
        unsigned char* dataToWrite = (unsigned char*)malloc(PADDED_SIZE);
        fopen_s(&myFile, caInputFileName, "rb");
        fread(dataToWrite, 1, PADDED_SIZE, myFile);
        fclose(myFile);

        // puis acces en ecriture
        HANDLE  hWriteVolume = getHandleOnVolume(volumeID, GENERIC_WRITE);
        DWORD   writeDeviceID = getDeviceID(hWriteVolume);
        HANDLE  hWriteRawDisk;

        if (!getLockOnVolume(hWriteVolume))
        {
            CloseHandle(hWriteVolume);
            exit(-1);
        }

        if (!unmountVolume(hWriteVolume))
        {
            removeLockOnVolume(hWriteVolume);
            CloseHandle(hWriteVolume);
            exit(-1);
        }

        hWriteRawDisk = getHandleOnDevice(writeDeviceID, GENERIC_WRITE);
        if (hWriteRawDisk == INVALID_HANDLE_VALUE)
        {
            removeLockOnVolume(hWriteVolume);
            //CloseHandle(hFile);
            CloseHandle(hWriteVolume);
            exit(-1);
        }

        fprintf(myLogFile, "\n\n\nDump dataToWrite :\n");
        //memset(dataToWrite + 256, 0, 256);                // juste un essai
        HexDump((unsigned char*)dataToWrite, 8 * 512);

        uint32_t alteraPartitionStartSector = *((uint32_t*)(sectorData + OFFSET_INFO_PARTITION1_IN_MBR + 8 + 16 * uiAlteraPartition));
        writeSectorDataToHandle(hWriteRawDisk, (char*)dataToWrite, alteraPartitionStartSector, PADDED_SIZE/512, 512ul);

        free(dataToWrite);
        removeLockOnVolume(hWriteVolume);
        CloseHandle(hWriteRawDisk);
        CloseHandle(hWriteVolume);

        fclose(myLogFile);
}

