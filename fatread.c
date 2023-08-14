#include <stdio.h>
#include <stdint.h>

#define BPB_START_OFFSET  11
#define BPB_SIZE_FAT12    0x3A
#define BPB_SIZE_FAT16    BPB_SIZE_FAT12



/* DOS 3.x BPB for FAT12 and FAT16 */
struct bpb_fat12 {
    uint16_t BytesPerSector;
    uint8_t  SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t  NumberOfFATs;
    uint16_t RootDirEntries;
    uint16_t TotalSectors16;
    uint8_t  MediaDescriptor;
    uint16_t SectorsPerFAT16;

    /* DOS 3.31 BPB */
    uint16_t SectorsPerTrack; /* Usually 9 or 18 */
    uint16_t NumHeads;        /* Usually 2       */
    uint32_t HiddenSectors;
    uint32_t TotalSectors32;

    /* DOS 3.4 BPB */
    uint8_t  DriveNumber;
    uint8_t  Reserved;
    uint8_t  ExtendedBootSignature;
    uint32_t VolumeSerial;
    char     VolumeLabel[11 + 1];
    char     FileSystemType[8 + 1];
};


/*
   print_fat12_info

   Print out the BPB information for FAT12 volume

 */
void print_fat12_info(struct bpb_fat12 *bpb) 
{
    printf("Volume Information for \"%s\":\n\n", bpb->VolumeLabel);
    printf("        BytesPerSector: %d\n", bpb->BytesPerSector);
    printf("     SectorsPerCluster: %d\n", bpb->SectorsPerCluster);
    printf("       ReservedSectors: %d\n", bpb->ReservedSectors);
    printf("          NumberOfFATs: %d\n", bpb->NumberOfFATs);
    printf("        RootDirEntries: %d\n", bpb->RootDirEntries);
    printf("        TotalSectors16: %d\n", bpb->TotalSectors16);
    printf("       MediaDescriptor: 0x%X\n", bpb->MediaDescriptor);
    printf("       SectorsPerFAT16: %d\n", bpb->SectorsPerFAT16);
    printf("       SectorsPerTrack: %d\n", bpb->SectorsPerTrack);
    printf("                 Heads: %d\n", bpb->NumHeads);
    printf("         HiddenSectors: %d\n", bpb->HiddenSectors);
    printf("           DriveNumber: %d\n", bpb->DriveNumber);
    printf("              Reserved: %d\n", bpb->Reserved);
    printf(" ExtendedBootSignature: 0x%02x\n", bpb->ExtendedBootSignature);
    printf("          VolumeSerial: %d\n", bpb->VolumeSerial);
    printf("           VolumeLabel: %11s\n", bpb->VolumeLabel);
    printf("        FileSystemType: %8s\n", bpb->FileSystemType);
}


/* 
   read_fat12_info
  
   The BPB structure will not have ALL fields filled.

   TotalSectors16 OR TotalSectors32 should be filled out, but not both.

*/
int read_fat12_info(struct bpb_fat12 *bpb, FILE *fp)
{
    fseek(fp, BPB_START_OFFSET, SEEK_SET);
    fread(&bpb->BytesPerSector, sizeof(uint16_t), 1, fp);
    fread(&bpb->SectorsPerCluster, sizeof(uint8_t), 1, fp);
    fread(&bpb->ReservedSectors, sizeof(uint16_t), 1, fp);
    fread(&bpb->NumberOfFATs, sizeof(uint8_t), 1, fp);
    fread(&bpb->RootDirEntries, sizeof(uint16_t), 1, fp);
    fread(&bpb->TotalSectors16, sizeof(uint16_t), 1, fp);
    fread(&bpb->MediaDescriptor, sizeof(uint8_t), 1, fp);
    fread(&bpb->SectorsPerFAT16, sizeof(uint16_t), 1, fp);

    /* DOS 3.31 BPB */
    fread(&bpb->SectorsPerTrack, sizeof(uint16_t), 1, fp);
    fread(&bpb->NumHeads, sizeof(uint16_t), 1, fp);
    fread(&bpb->HiddenSectors, sizeof(uint32_t), 1, fp);
    fread(&bpb->TotalSectors32, sizeof(uint32_t), 1, fp);

    /* DOS 3.4 BPB */
    fread(&bpb->DriveNumber, sizeof(uint8_t), 1, fp);
    fread(&bpb->Reserved, sizeof(uint8_t), 1, fp);
    fread(&bpb->ExtendedBootSignature, sizeof(uint8_t), 1, fp);
    fread(&bpb->VolumeSerial, sizeof(uint32_t), 1, fp);
    fread(&bpb->VolumeLabel, sizeof(char), 11, fp);
    fread(&bpb->FileSystemType, sizeof(char), 8, fp);
    bpb->VolumeLabel[11] = '\0';
    bpb->FileSystemType[8] = '\0';


    /* Check to make sure we just read a valid BPB... */

    
    if (bpb->BytesPerSector != 512) {
        return -1;
    }

    /* If totalsectors16 is 0, then totalsectors32 must be non-zero */
    if (bpb->TotalSectors16 == 0 && bpb->TotalSectors32 == 0) {
        return -1;
    }

    /* If totalsectors16 is non-zero, then totalsectors32 must be 0 */
    if (bpb->TotalSectors16 != 0 && bpb->TotalSectors32 != 0) {
        return -1;
    }

    return 0;
}



int main(int argc, char **argv) 
{
    FILE *fp;
    struct bpb_fat12 bpb;
    int valid_fat = 0;
    
    if (argc < 2) {
        fp = stdin;
    } else if (argc == 2) {
        fp = fopen(argv[1], "rb");
        if (fp == NULL) {
            printf("Error: could not open %s\n", argv[1]);
            return -1;
        }
    } else {
        printf("Usage: fatread [disk_image]\n");
        return -1;
    }
    
    valid_fat = read_fat12_info(&bpb, fp);
    fclose(fp);
    
    print_fat12_info(&bpb);
    
    return valid_fat;
}
