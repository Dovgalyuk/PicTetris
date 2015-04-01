
static const unsigned char digits[10][8] = 
 {
  {0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00}, //0
  {0x00, 0x18, 0x28, 0x08, 0x08, 0x08, 0x3e, 0x00}, //1
  {0x00, 0x3c, 0x42, 0x02, 0x3c, 0x40, 0x7e, 0x00}, //2
  {0x00, 0x3c, 0x42, 0x0c, 0x02, 0x42, 0x3c, 0x00}, //3
  {0x00, 0x08, 0x18, 0x28, 0x48, 0x7e, 0x08, 0x00}, //4
  {0x00, 0x7e, 0x40, 0x7c, 0x02, 0x42, 0x3c, 0x00}, //5
  {0x00, 0x3c, 0x40, 0x7c, 0x42, 0x42, 0x3c, 0x00}, //6
  {0x00, 0x7e, 0x02, 0x04, 0x08, 0x10, 0x10, 0x00}, //7
  {0x00, 0x3c, 0x42, 0x3c, 0x42, 0x42, 0x3c, 0x00}, //8
  {0x00, 0x3c, 0x42, 0x42, 0x3e, 0x02, 0x3c, 0x00}, //9
 };


unsigned char GetDigitLine(unsigned char digit, unsigned char line)
{
  return digits[digit][line];
}
