#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include "resource.h"

HINSTANCE hInst;
HWND hWnd;

const int base = 0x378;

LARGE_INTEGER timerFreq;


// типы команд
const int CMD_CORE = 0x0;
const int CMD_SHIFT_OUT_TABLAT = 0x2;
const int CMD_TABLE_READ = 0x8;
const int CMD_TABLE_READ_POST_INC = 0x9;
const int CMD_TABLE_READ_POST_DEC = 0xa;
const int CMD_TABLE_READ_PRE_INC = 0xb;
const int CMD_TABLE_WRITE = 0xc;
const int CMD_TABLE_WRITE_POST_INC2 = 0xd;
const int CMD_TABLE_WRITE_POST_DEC2 = 0xe;
const int CMD_TABLE_WRITE_START = 0xf;

// команды стирания
const int ERASE_CHIP = 0x80;
const int ERASE_EEPROM = 0x81;
const int ERASE_BOOT = 0x83;
const int ERASE_PANEL1 = 0x88;
const int ERASE_PANEL2 = 0x89;
const int ERASE_PANEL3 = 0x8a;
const int ERASE_PANEL4 = 0x8b;

// задержки
const int TIME_P2 = 100;
const int TIME_P2a = 40;
const int TIME_P2b = 40;
const int TIME_P3 = 15;
const int TIME_P4 = 15;
const int TIME_P5 = 20;
const int TIME_P5a = 20;
const int TIME_P6 = 20;
const int TIME_P9 = 1000000;
const int TIME_P10 = 5000;
const int TIME_P11 = 5000000;
const int TIME_P12 = 2000;
const int TIME_P13 = 100;
const int TIME_P14 = 10;
const int TIME_P15 = 2000;

// память
const int EEPROM_SIZE = 256;
BYTE EEPROM[EEPROM_SIZE];

const int CODE_SIZE = 0x400000;
BYTE CODE[CODE_SIZE];
bool CodeValid[CODE_SIZE];

const int ID_START = 0x200000;
const int ID_END = 0x200007;

const int CONFIG_START = 0x300000;
const int CONFIG_END = 0x30000d;

const int DEVICE_ID_START = 0x3ffffe;
const int DEVICE_ID_END = 0x3fffff;

const int MEM_BLOCKS_NUM = 5;
const int MEM_BLOCKS_BEGIN[MEM_BLOCKS_NUM + 1] = 
    {0, 0x200, 0x2000, 0x4000, 0x6000, 0x8000};
const int MEM_BLOCKS_ERASE[MEM_BLOCKS_NUM] =
    {ERASE_BOOT, ERASE_PANEL1, ERASE_PANEL2, ERASE_PANEL3, ERASE_PANEL4};
const int PROGRAM_SIZE = 0x8000;
const int PROGRAM_START = 0x0000;
const int PROGRAM_END = 0x7fff;

// слово конфигурации
enum BitNames
{
    FOSC0 = 0,
    FOSC1,
    FOSC2,
    OSCEN,
    PWRTE,
    BOREN,
    BORV2,
    BORV1,
    WDTEN,
    WDTPS0,
    WDTPS1,
    WDTPS2,
    CCP2MX,
    STVREN,
    LVP,
    BKBUG,
    CP0,
    CP1,
    CP2,
    CP3,
    CPB,
    CPD,
    WRT0,
    WRT1,
    WRT2,
    WRT3,
    WRTC,
    WRTB,
    WRTD,
    EBTR0,
    EBTR1,
    EBTR2,
    EBTR3,
    EBTRB,
    BitsNum
};
const int bitsAddr[] = 
{
    0x1*8 + 0,//FOSC0, 
    0x1*8 + 1,//FOSC1,
    0x1*8 + 2,//FOSC2,
    0x1*8 + 5,//OSCEN,
    0x2*8 + 0,//PWRTE,
    0x2*8 + 1,//BOREN,
    0x2*8 + 2,//BORV2,
    0x2*8 + 3,//BORV1,
    0x3*8 + 0,//WDTEN,
    0x3*8 + 1,//WDTPS0,
    0x3*8 + 2,//WDTPS1,
    0x3*8 + 3,//WDTPS2,
    0x5*8 + 0,//CCP2MX,
    0x6*8 + 0,//STVREN,
    0x6*8 + 2,//LVP,
    0x6*8 + 7,//BKBUG,
    0x8*8 + 0,//CP0,
    0x8*8 + 1,//CP1,
    0x8*8 + 2,//CP2,
    0x8*8 + 3,//CP3,
    0x9*8 + 6,//CPB,
    0x9*8 + 7,//CPD,
    0xa*8 + 0,//WRT0,
    0xa*8 + 1,//WRT1,
    0xa*8 + 2,//WRT2,
    0xa*8 + 3,//WRT3,
    0xb*8 + 5,//WRTC,
    0xb*8 + 6,//WRTB,
    0xb*8 + 7,//WRTD,
    0xc*8 + 0,//EBTR0,
    0xc*8 + 1,//EBTR1,
    0xc*8 + 2,//EBTR2,
    0xc*8 + 3,//EBTR3,
    0xd*8 + 6,//EBTRB
};

#define BitControl(name) IDC_##name
#define ReadConfigBit(name) \
do \
{ \
    int addr = CONFIG_START + bitsAddr[name] / 8; \
    CODE[addr] = CODE[addr] & ~(1 << (bitsAddr[name] & 7)); \
    CODE[addr] = CODE[addr] | ((IsDlgButtonChecked(hWnd, BitControl(name)) == BST_CHECKED ? 1 : 0) << (bitsAddr[name] & 7)); \
} while(0)
#define SetConfigBit(name) \
do \
{ \
    int addr = CONFIG_START + bitsAddr[name] / 8; \
    CheckDlgButton(hWnd, BitControl(name), ((CODE[addr] >> (bitsAddr[name] & 7)) & 1) ? BST_CHECKED : BST_UNCHECKED);\
} while(0)

void GetBitsState()
{
    ReadConfigBit(FOSC0);
    ReadConfigBit(FOSC1);
    ReadConfigBit(FOSC2);
    ReadConfigBit(OSCEN);
    ReadConfigBit(PWRTE);
    ReadConfigBit(BOREN);
    ReadConfigBit(BORV2);
    ReadConfigBit(BORV1);
    ReadConfigBit(WDTEN);
    ReadConfigBit(WDTPS0);
    ReadConfigBit(WDTPS1);
    ReadConfigBit(WDTPS2);
    ReadConfigBit(CCP2MX);
    ReadConfigBit(STVREN);
    ReadConfigBit(LVP);
    ReadConfigBit(BKBUG);
    ReadConfigBit(CP0);
    ReadConfigBit(CP1);
    ReadConfigBit(CP2);
    ReadConfigBit(CP3);
    ReadConfigBit(CPB);
    ReadConfigBit(CPD);
    ReadConfigBit(WRT0);
    ReadConfigBit(WRT1);
    ReadConfigBit(WRT2);
    ReadConfigBit(WRT3);
    ReadConfigBit(WRTC);
    ReadConfigBit(WRTB);
    ReadConfigBit(WRTD);
    ReadConfigBit(EBTR0);
    ReadConfigBit(EBTR1);
    ReadConfigBit(EBTR2);
    ReadConfigBit(EBTR3);
    ReadConfigBit(EBTRB);
}

void SetBitsState()
{
    SetConfigBit(FOSC0);
    SetConfigBit(FOSC1);
    SetConfigBit(FOSC2);
    SetConfigBit(OSCEN);
    SetConfigBit(PWRTE);
    SetConfigBit(BOREN);
    SetConfigBit(BORV2);
    SetConfigBit(BORV1);
    SetConfigBit(WDTEN);
    SetConfigBit(WDTPS0);
    SetConfigBit(WDTPS1);
    SetConfigBit(WDTPS2);
    SetConfigBit(CCP2MX);
    SetConfigBit(STVREN);
    SetConfigBit(LVP);
    SetConfigBit(BKBUG);
    SetConfigBit(CP0);
    SetConfigBit(CP1);
    SetConfigBit(CP2);
    SetConfigBit(CP3);
    SetConfigBit(CPB);
    SetConfigBit(CPD);
    SetConfigBit(WRT0);
    SetConfigBit(WRT1);
    SetConfigBit(WRT2);
    SetConfigBit(WRT3);
    SetConfigBit(WRTC);
    SetConfigBit(WRTB);
    SetConfigBit(WRTD);
    SetConfigBit(EBTR0);
    SetConfigBit(EBTR1);
    SetConfigBit(EBTR2);
    SetConfigBit(EBTR3);
    SetConfigBit(EBTRB);
}








void Pause(double time) // time in nanoseconds
{
    LARGE_INTEGER t, t0;
    double dt = (time * (double)timerFreq.QuadPart) / 1000000000.0;
    
    QueryPerformanceCounter(&t0);

    do
    {
        QueryPerformanceCounter(&t);
        double dt1 = (double)t.QuadPart - (double)t0.QuadPart;
        if (dt1 < 0)
        {
            LONGLONG d = ((LONGLONG)1) << 64 - 1;
            dt1 += d;
        }
        if (dt < dt1)
            break;
    }
    while (1);
}


void ResetArrays()
{
    memset(EEPROM, 0xff, EEPROM_SIZE);
    memset(CODE, 0xff, CODE_SIZE);
    memset(CodeValid, 0, CODE_SIZE * sizeof(bool));
}



void PushMessage(char *s)
{
    SendDlgItemMessage(hWnd, IDC_MESSAGES, LB_ADDSTRING, 0, (LPARAM)s);
}





void ResetPort()
{
    _outp(base, 0xff);
}


void BeginProgramming()
{
    _outp(base, _inp(base) & ~4); // VDD
    Pause(TIME_P13);
    _outp(base, _inp(base) & ~8); // VPP
    Pause(TIME_P12);
}


void SendBit(int bit)
{
    // set data to bit and clock to 1
    _outp(base, (_inp(base) & 0xfc) | ((~bit) & 1));
    // max(P2a, P3)
    Pause(max(TIME_P2a, TIME_P3));
    // set clock to 0
    _outp(base, _inp(base) | 2);
    // max(P2b, P4)
    Pause(max(TIME_P2b, TIME_P4));
}


int ReadBit()
{
    int r;
    // set clock to 1
    _outp(base, _inp(base) & 0xfd);
    // max(P2a, P14)
    Pause(max(TIME_P2a, TIME_P14));
    // read
    r = (~(_inp(base + 1) >> 6)) & 1;
    // set clock to 0
    _outp(base, _inp(base) | 2);
    // P2b
    Pause(TIME_P2b);

    return r;
}



void SendCommand(int command, int data)
{
    int cmd = command;
    for (int i = 0 ; i < 4 ; i++, cmd >>= 1)
        SendBit(cmd & 1);
    // P5
    Pause(TIME_P5);
    for (i = 0 ; i < 16 ; i++, data >>= 1)
        SendBit(data & 1);
    // P5a
    Pause(TIME_P5a);
}



int SendCommandAndGetResult(int command, int data)
{
    int r = 0;
    
    int cmd = command;
    for (int i = 0 ; i < 4 ; i++, cmd >>= 1)
        SendBit(cmd & 1);

    // P5
    Pause(TIME_P5);
    for (i = 0 ; i < 8 ; i++, data >>= 1)
        SendBit(data & 1);
    // P6
    Pause(TIME_P6);

    // reading
    // set data line to 1
    _outp(base, _inp(base) & 0xfe);
    // read bits
    for (i = 0 ; i < 8 ; i++)
        r = r | ((ReadBit() & 1) << i);

    // P5a
    Pause(TIME_P5a);

    return r;
}


void BulkErase(int area)
{
    BeginProgramming();

    SendCommand(CMD_CORE, 0x0e3c);
    SendCommand(CMD_CORE, 0x6ef8);
    SendCommand(CMD_CORE, 0x0e00);
    SendCommand(CMD_CORE, 0x6ef7);
    SendCommand(CMD_CORE, 0x0e04);
    SendCommand(CMD_CORE, 0x6ef6);
    SendCommand(CMD_TABLE_WRITE, area);
    SendCommand(CMD_CORE, 0x0000);

    for (int i = 0 ; i < 4 ; i++)
        SendBit(0);

    Pause(TIME_P11);
    Pause(TIME_P10);

    for (i = 0 ; i < 16 ; i++)
        SendBit(0);

    ResetPort();
}



void ReadEEPROM()
{
    // enter programming mode
    BeginProgramming();

    SendCommand(CMD_CORE, 0x9ea6);
    SendCommand(CMD_CORE, 0x9ca6);
    for (int addr = 0 ; addr < EEPROM_SIZE ; addr++)
    {
        // set addr
        SendCommand(CMD_CORE, 0x0e00 + addr);
        SendCommand(CMD_CORE, 0x6ea9);
        // initiate memread
        SendCommand(CMD_CORE, 0x80a6);
        // load data and shift out
        SendCommand(CMD_CORE, 0x50a8);
        SendCommand(CMD_CORE, 0x6ef5);
        EEPROM[addr] = SendCommandAndGetResult(CMD_SHIFT_OUT_TABLAT, 0);
    }

    PushMessage("--- EEPROM Read ok ---");

    // end program
    ResetPort();
}



void CheckEEPROM()
{
    bool ok = true;
    // enter programming mode
    BeginProgramming();

    SendCommand(CMD_CORE, 0x9ea6);
    SendCommand(CMD_CORE, 0x9ca6);
    for (int addr = 0 ; ok && addr < EEPROM_SIZE ; addr++)
    {
        // set addr
        SendCommand(CMD_CORE, 0x0e00 + addr);
        SendCommand(CMD_CORE, 0x6ea9);
        // initiate memread
        SendCommand(CMD_CORE, 0x80a6);
        // load data and shift out
        SendCommand(CMD_CORE, 0x50a8);
        SendCommand(CMD_CORE, 0x6ef5);
        ok = EEPROM[addr] == SendCommandAndGetResult(CMD_SHIFT_OUT_TABLAT, 0);
    }

    if (ok)
        PushMessage("--- EEPROM is correct ---");
    else
        PushMessage("--- EEPROM is incorrect ---");

    // end program
    ResetPort();
}




void WriteEEPROM()
{
    BulkErase(ERASE_EEPROM);

    int i;
    // enter programming mode
    BeginProgramming();

    // direct access to EEPROM
    SendCommand(CMD_CORE, 0x9ea6);
    SendCommand(CMD_CORE, 0x9ca6);
    for (int addr = 0 ; addr < EEPROM_SIZE ; addr++)
    //int addr = 0;
    {
        // set addr
        SendCommand(CMD_CORE, 0x0e00 + addr);
        SendCommand(CMD_CORE, 0x6ea9);
        // set data
        SendCommand(CMD_CORE, 0x0e00 + EEPROM[addr]);
        SendCommand(CMD_CORE, 0x6ea8);

        SendCommand(CMD_CORE, 0x84a6);

        SendCommand(CMD_CORE, 0x0e55);
        SendCommand(CMD_CORE, 0x6ea7);
        SendCommand(CMD_CORE, 0x0eaa);
        SendCommand(CMD_CORE, 0x6ea7);

        SendCommand(CMD_CORE, 0x82a6);
        SendCommand(CMD_CORE, 0);

        for (i = 0 ; i < 4 ; i++)
            SendBit(0);

        Pause(TIME_P11);
        Pause(TIME_P10);

        for (i = 0 ; i < 16 ; i++)
            SendBit(0);

        SendCommand(CMD_CORE, 0x94a6);
    }
    PushMessage("--- EEPROM written ok ---");

    // end program
    ResetPort();
}





void ReadCODE(int start, int end)
{
    // enter programming mode
    BeginProgramming();

    SendCommand(CMD_CORE, 0x0e00 + ((start >> 16) & 0xff));
    SendCommand(CMD_CORE, 0x6ef8);
    SendCommand(CMD_CORE, 0x0e00 + ((start >> 8) & 0xff));
    SendCommand(CMD_CORE, 0x6ef7);
    SendCommand(CMD_CORE, 0x0e00);
    SendCommand(CMD_CORE, 0x6ef6 + (start & 0xff));
    for (int addr = start ; addr <= end ; addr++)
    {
        CODE[addr] = SendCommandAndGetResult(CMD_TABLE_READ_POST_INC, 0);
    }

    char s[100];
    sprintf(s, "--- CODE from %x to %x read ok ---", start, end);
    PushMessage(s);

    // end program
    ResetPort();
}



void WriteProgram()
{
    int i, addr;
    // first erase
    for (i = 0 ; i < MEM_BLOCKS_NUM ; i++)
        for (addr = MEM_BLOCKS_BEGIN[i] ; addr < MEM_BLOCKS_BEGIN[i + 1] ; addr++)
        {
            if (CodeValid[addr])
            {
                BulkErase(MEM_BLOCKS_ERASE[i]);
                break;
            }
        }

    BeginProgramming();

    // configure device for single-panel writes
    SendCommand(CMD_CORE, 0x0e3c);
    SendCommand(CMD_CORE, 0x6ef8);
    SendCommand(CMD_CORE, 0x0e00);
    SendCommand(CMD_CORE, 0x6ef7);
    SendCommand(CMD_CORE, 0x0e06);
    SendCommand(CMD_CORE, 0x6ef6);
    SendCommand(CMD_TABLE_WRITE, 0x0000);
    // enable direct access to code memory
    SendCommand(CMD_CORE, 0x8ea6);
    SendCommand(CMD_CORE, 0x9ca6);

    addr = 0;
    while (addr < PROGRAM_SIZE && !CodeValid[addr])
        addr++;
    // NOT TESTED
    addr -= addr & 7;
    //
    while (addr < PROGRAM_SIZE/*0x200*/)
    {
        SendCommand(CMD_CORE, 0x0e00 + 0);
        SendCommand(CMD_CORE, 0x6ef8);
        SendCommand(CMD_CORE, 0x0e00 + ((addr >> 8) & 0xff));
        SendCommand(CMD_CORE, 0x6ef7);
        SendCommand(CMD_CORE, 0x0e00 + (addr & 0xff));
        SendCommand(CMD_CORE, 0x6ef6);

        SendCommand(CMD_TABLE_WRITE_POST_INC2, CODE[addr] + (int)CODE[addr + 1] * 0x100);
        addr += 2;
        SendCommand(CMD_TABLE_WRITE_POST_INC2, CODE[addr] + (int)CODE[addr + 1] * 0x100);
        addr += 2;
        SendCommand(CMD_TABLE_WRITE_POST_INC2, CODE[addr] + (int)CODE[addr + 1] * 0x100);
        addr += 2;
        SendCommand(CMD_TABLE_WRITE_START, CODE[addr] + (int)CODE[addr + 1] * 0x100);
        addr += 2;

        for (i = 0 ; i < 3 ; i++)
            SendBit(0);

        // set data to bit and clock to 1
        _outp(base, (_inp(base) & 0xfc) | 1);

        Pause(TIME_P9);

        // set clock to 0
        _outp(base, _inp(base) | 2);

        Pause(TIME_P10);

        for (i = 0 ; i < 16 ; i++)
            SendBit(0);

        while (addr < PROGRAM_SIZE && !CodeValid[addr])
            addr++;
        
        // NOT TESTED
        addr -= addr & 7;
        //
    }

    ResetPort();

    PushMessage("--- Program written ok ---");
}


void WriteConfig()
{
    int addr;
    
    GetBitsState();

    BeginProgramming();

    // configure device for single-panel writes
    SendCommand(CMD_CORE, 0x0e3c);
    SendCommand(CMD_CORE, 0x6ef8);
    SendCommand(CMD_CORE, 0x0e00);
    SendCommand(CMD_CORE, 0x6ef7);
    SendCommand(CMD_CORE, 0x0e06);
    SendCommand(CMD_CORE, 0x6ef6);
    SendCommand(CMD_TABLE_WRITE, 0x0000);
    // enable direct access to code memory
    SendCommand(CMD_CORE, 0x8ea6);
    SendCommand(CMD_CORE, 0x8ca6);

    SendCommand(CMD_CORE, 0x0e30);
    SendCommand(CMD_CORE, 0x6ef8);
    SendCommand(CMD_CORE, 0x0e00);
    SendCommand(CMD_CORE, 0x6ef7);
    SendCommand(CMD_CORE, 0x0e00);
    SendCommand(CMD_CORE, 0x6ef6);
    for (addr = CONFIG_START ; addr <= CONFIG_END ; addr++)
    {
        SendCommand(CMD_TABLE_WRITE_START, (int)CODE[addr] << (8 * (addr & 1)));

        for (int i = 0 ; i < 3 ; i++)
            SendBit(0);
        // set data to bit and clock to 1
        _outp(base, (_inp(base) & 0xfc) | 1);

        Pause(TIME_P9);
        
        // set clock to 0
        _outp(base, _inp(base) | 2);

        Pause(TIME_P10);

        for (i = 0 ; i < 16 ; i++)
            SendBit(0);

        // next addr
        SendCommand(CMD_CORE, 0x2af6);
    }

    ResetPort();

    PushMessage("--- Config written ok ---");
}





void SaveCODE(const char *fname, int start, int end)
{
    FILE *f = fopen(fname, "wb");
    if (f)
    {
        fwrite(CODE + start, 1, end - start + 1, f);
        fclose(f);

        char s[1000];
        sprintf(s, "--- Memory from %x to %x written to %s successfully ---", start, end, fname);
        PushMessage(s);
    }
}



void SaveEEPROM(const char *fname)
{
    FILE *f = fopen(fname, "wb");
    if (f)
    {
        fwrite(EEPROM, 1, EEPROM_SIZE, f);
        fclose(f);

        char s[1000];
        sprintf(s, "--- EEPROM written to %s successfully ---", fname);
        PushMessage(s);
    }
}




void PrintID()
{
    char s[100];
    char *ss = s;
    sprintf(ss, "ID:");
    ss += strlen(ss);
    for (int addr = ID_START ; addr <= ID_END ; addr++)
    {
        sprintf(ss, " %x", CODE[addr]);
        ss += strlen(ss);
    }
    PushMessage(s);
}


void PrintConfig()
{
    char s[100];
    char *ss = s;
    sprintf(ss, "Config:");
    ss += strlen(ss);
    for (int addr = CONFIG_START ; addr <= CONFIG_END ; addr++)
    {
        sprintf(ss, " %x", CODE[addr]);
        ss += strlen(ss);
    }
    PushMessage(s);
}


void PrintDeviceID()
{
    char s[100];
    char *ss = s;
    sprintf(ss, "Device ID:");
    ss += strlen(ss);
    for (int addr = DEVICE_ID_START ; addr <= DEVICE_ID_END ; addr++)
    {
        sprintf(ss, " %x", CODE[addr]);
        ss += strlen(ss);
    }
    PushMessage(s);
}





int HtoD(int c)
{
    return c >= '0' && c <= '9' ? c - '0' : c - 'A' + 10;
}

void OnOpenFile()
{
    char s[1000];
    s[0] = 0;
    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = "Hex files\0*.HEX\0";
    ofn.lpstrFile = s;
    ofn.nMaxFile = 1000;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileName(&ofn))
    {
        FILE *f = fopen(s, "rt");
        if (!f)
            return;
        while (!feof(f))
        {
            int len, addr;
            fgetc(f);
            len = HtoD(fgetc(f)) * 0x10 + HtoD(fgetc(f));
            if (!len)
                break;
            addr = HtoD(fgetc(f)) * 0x1000 + HtoD(fgetc(f)) * 0x100 + HtoD(fgetc(f)) * 0x10 + HtoD(fgetc(f));
            fgetc(f);
            fgetc(f);
            while (len--)
            {
                CODE[addr] = HtoD(fgetc(f)) * 0x10 + HtoD(fgetc(f));
                CodeValid[addr] = true;
                addr++;
            }
            while (!feof(f) && fgetc(f) != '\n')
                ;
        }
        fclose(f);

        char ss[1100];
        sprintf(ss, "--- File %s read ok ---", s);
        PushMessage(ss);
    }
}











BOOL CALLBACK DlgProc(HWND hwndDlg, UINT uMsg,
                         WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        hWnd = hwndDlg;
        ResetPort();
        ResetArrays();
        QueryPerformanceFrequency(&timerFreq);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_READEEPROM:
            ReadEEPROM();
            break;
        case IDM_READID:
            ReadCODE(ID_START, ID_END);
            PrintID();
            break;
        case IDM_READDEVICEID:
            ReadCODE(DEVICE_ID_START, DEVICE_ID_END);
            PrintDeviceID();
            break;
        case IDM_READCONFIG:
            ReadCODE(CONFIG_START, CONFIG_END);
            PrintConfig();
            SetBitsState();
            break;
        case IDM_READPROGRAM:
            ReadCODE(PROGRAM_START, PROGRAM_END);
            break;
        case IDM_WRITEPROGRAM:
            WriteProgram();
            break;
        case IDM_WRITEEEPROM:
            WriteEEPROM();
            break;
        case IDM_WRITECONFIG:
            WriteConfig();
            break;
        case IDM_SAVEPROGRAM:
            SaveCODE("program.bin", PROGRAM_START, PROGRAM_END);
            break;
        case IDM_SAVEEEPROM:
            SaveEEPROM("eeprom.bin");
            break;
        case IDM_OPENFILE:
            OnOpenFile();
            break;
        case IDM_ERASEPROGRAM:
            BulkErase(ERASE_BOOT);
            BulkErase(ERASE_PANEL1);
            BulkErase(ERASE_PANEL2);
            BulkErase(ERASE_PANEL3);
            BulkErase(ERASE_PANEL4);
            PushMessage("--- Program memory erased ---");
            break;
        }
        break;
    case WM_CLOSE:
        EndDialog(hwndDlg, 0);
        break;
    }
    return FALSE;
}






int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    hInst = hInstance;
    
    return DialogBox(hInst, MAKEINTRESOURCE(IDD_MAIN), 0, (DLGPROC)DlgProc);
}



