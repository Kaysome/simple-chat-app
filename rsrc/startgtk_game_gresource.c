#include <gio/gio.h>

#if defined (__ELF__) && ( __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 6))
# define SECTION __attribute__ ((section (".gresource.startgtk"), aligned (8)))
#else
# define SECTION
#endif

#ifdef _MSC_VER
static const SECTION union { const guint8 data[46369]; const double alignment; void * const ptr;}  startgtk_resource_data = { {
  0107, 0126, 0141, 0162, 0151, 0141, 0156, 0164, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, 
  0030, 0000, 0000, 0000, 0164, 0000, 0000, 0000, 0000, 0000, 0000, 0050, 0003, 0000, 0000, 0000, 
  0000, 0000, 0000, 0000, 0002, 0000, 0000, 0000, 0002, 0000, 0000, 0000, 0363, 0303, 0304, 0167, 
  0002, 0000, 0000, 0000, 0164, 0000, 0000, 0000, 0016, 0000, 0166, 0000, 0210, 0000, 0000, 0000, 
  0123, 0216, 0000, 0000, 0161, 0236, 0026, 0117, 0002, 0000, 0000, 0000, 0123, 0216, 0000, 0000, 
  0013, 0000, 0166, 0000, 0140, 0216, 0000, 0000, 0026, 0265, 0000, 0000, 0324, 0265, 0002, 0000, 
  0377, 0377, 0377, 0377, 0026, 0265, 0000, 0000, 0001, 0000, 0114, 0000, 0030, 0265, 0000, 0000, 
  0040, 0265, 0000, 0000, 0163, 0164, 0141, 0162, 0164, 0147, 0164, 0153, 0056, 0147, 0154, 0141, 
  0144, 0145, 0000, 0000, 0000, 0000, 0000, 0000, 0273, 0215, 0000, 0000, 0000, 0000, 0000, 0000, 
  0074, 0077, 0170, 0155, 0154, 0040, 0166, 0145, 0162, 0163, 0151, 0157, 0156, 0075, 0042, 0061, 
  0056, 0060, 0042, 0040, 0145, 0156, 0143, 0157, 0144, 0151, 0156, 0147, 0075, 0042, 0125, 0124, 
  0106, 0055, 0070, 0042, 0077, 0076, 0012, 0074, 0041, 0055, 0055, 0040, 0107, 0145, 0156, 0145, 
  0162, 0141, 0164, 0145, 0144, 0040, 0167, 0151, 0164, 0150, 0040, 0147, 0154, 0141, 0144, 0145, 
  0040, 0063, 0056, 0062, 0062, 0056, 0061, 0040, 0055, 0055, 0076, 0012, 0074, 0151, 0156, 0164, 
  0145, 0162, 0146, 0141, 0143, 0145, 0076, 0012, 0040, 0040, 0074, 0162, 0145, 0161, 0165, 0151, 
  0162, 0145, 0163, 0040, 0154, 0151, 0142, 0075, 0042, 0147, 0164, 0153, 0053, 0042, 0040, 0166, 
  0145, 0162, 0163, 0151, 0157, 0156, 0075, 0042, 0063, 0056, 0064, 0042, 0057, 0076, 0012, 0040, 
  0040, 0074, 0157, 0142, 0152, 0145, 0143, 0164, 0040, 0143, 0154, 0141, 0163, 0163, 0075, 0042, 
  0107, 0164, 0153, 0114, 0151, 0163, 0164, 0123, 0164, 0157, 0162, 0145, 0042, 0040, 0151, 0144, 
  0075, 0042, 0147, 0141, 0155, 0145, 0154, 0151, 0163, 0164, 0042, 0076, 0012, 0040, 0040, 0040, 
  0040, 0074, 0143, 0157, 0154, 0165, 0155, 0156, 0163, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 
  0040, 0074, 0041, 0055, 0055, 0040, 0143, 0157, 0154, 0165, 0155, 0156, 0055, 0156, 0141, 0155, 
  0145, 0040, 0147, 0141, 0155, 0145, 0156, 0141, 0155, 0145, 0040, 0055, 0055, 0076, 0012, 0040, 
  0040, 0040, 0040, 0040, 0040, 0074, 0143, 0157, 0154, 0165, 0155, 0156, 0040, 0164, 0171, 0160, 
  0145, 0075, 0042, 0147, 0143, 0150, 0141, 0162, 0141, 0162, 0162, 0141, 0171, 0042, 0057, 0076, 
  0012, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0041, 0055, 0055, 0040, 0143, 0157, 0154, 0165, 
  0155, 0156, 0055, 0156, 0141, 0155, 0145, 0040, 0147, 0162, 0160, 0156, 0141, 0155, 0145, 0040, 
  0055, 0055, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0143, 0157, 0154, 0165, 0155, 
  0156, 0040, 0164, 0171, 0160, 0145, 0075, 0042, 0147, 0143, 0150, 0141, 0162, 0141, 0162, 0162, 
  0141, 0171, 0042, 0057, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0041, 0055, 0055, 
  0040, 0143, 0157, 0154, 0165, 0155, 0156, 0055, 0156, 0141, 0155, 0145, 0040, 0144, 0141, 0164, 
  0141, 0040, 0055, 0055, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0143, 0157, 0154, 
  0165, 0155, 0156, 0040, 0164, 0171, 0160, 0145, 0075, 0042, 0147, 0160, 0157, 0151, 0156, 0164, 
  0145, 0162, 0042, 0057, 0076, 0012, 0040, 0040, 0040, 0040, 0074, 0057, 0143, 0157, 0154, 0165, 
  0155, 0156, 0163, 0076, 0012, 0040, 0040, 0074, 0057, 0157, 0142, 0152, 0145, 0143, 0164, 0076, 
  0012, 0040, 0040, 0074, 0157, 0142, 0152, 0145, 0143, 0164, 0040, 0143, 0154, 0141, 0163, 0163, 
  0075, 0042, 0107, 0164, 0153, 0101, 0144, 0152, 0165, 0163, 0164, 0155, 0145, 0156, 0164, 0042, 
  0040, 0151, 0144, 0075, 0042, 0156, 0165, 0155, 0160, 0154, 0141, 0171, 0145, 0162, 0163, 0141, 
  0144, 0152, 0165, 0163, 0164, 0155, 0145, 0156, 0164, 0042, 0076, 0012, 0040, 0040, 0040, 0040, 
  0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 
  0154, 0157, 0167, 0145, 0162, 0042, 0076, 0062, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 
  0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 
  0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0165, 0160, 0160, 0145, 0162, 0042, 0076, 0061, 
  0066, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 
  0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 
  0042, 0166, 0141, 0154, 0165, 0145, 0042, 0076, 0062, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 
  0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 
  0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0163, 0164, 0145, 0160, 0137, 0151, 0156, 
  0143, 0162, 0145, 0155, 0145, 0156, 0164, 0042, 0076, 0061, 0074, 0057, 0160, 0162, 0157, 0160, 
  0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0074, 0057, 0157, 0142, 0152, 0145, 0143, 0164, 
  0076, 0012, 0040, 0040, 0074, 0157, 0142, 0152, 0145, 0143, 0164, 0040, 0143, 0154, 0141, 0163, 
  0163, 0075, 0042, 0107, 0164, 0153, 0114, 0151, 0163, 0164, 0123, 0164, 0157, 0162, 0145, 0042, 
  0040, 0151, 0144, 0075, 0042, 0163, 0157, 0165, 0156, 0144, 0161, 0165, 0141, 0154, 0151, 0164, 
  0171, 0154, 0151, 0163, 0164, 0042, 0076, 0012, 0040, 0040, 0040, 0040, 0074, 0143, 0157, 0154, 
  0165, 0155, 0156, 0163, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0041, 0055, 0055, 
  0040, 0143, 0157, 0154, 0165, 0155, 0156, 0055, 0156, 0141, 0155, 0145, 0040, 0155, 0157, 0144, 
  0145, 0040, 0055, 0055, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0143, 0157, 0154, 
  0165, 0155, 0156, 0040, 0164, 0171, 0160, 0145, 0075, 0042, 0147, 0143, 0150, 0141, 0162, 0141, 
  0162, 0162, 0141, 0171, 0042, 0057, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0041, 
  0055, 0055, 0040, 0143, 0157, 0154, 0165, 0155, 0156, 0055, 0156, 0141, 0155, 0145, 0040, 0151, 
  0156, 0144, 0145, 0170, 0040, 0055, 0055, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 
  0143, 0157, 0154, 0165, 0155, 0156, 0040, 0164, 0171, 0160, 0145, 0075, 0042, 0147, 0151, 0156, 
  0164, 0042, 0057, 0076, 0012, 0040, 0040, 0040, 0040, 0074, 0057, 0143, 0157, 0154, 0165, 0155, 
  0156, 0163, 0076, 0012, 0040, 0040, 0074, 0057, 0157, 0142, 0152, 0145, 0143, 0164, 0076, 0012, 
  0040, 0040, 0074, 0157, 0142, 0152, 0145, 0143, 0164, 0040, 0143, 0154, 0141, 0163, 0163, 0075, 
  0042, 0107, 0164, 0153, 0114, 0151, 0163, 0164, 0123, 0164, 0157, 0162, 0145, 0042, 0040, 0151, 
  0144, 0075, 0042, 0166, 0155, 0157, 0144, 0145, 0063, 0144, 0154, 0151, 0163, 0164, 0042, 0076, 
  0012, 0040, 0040, 0040, 0040, 0074, 0143, 0157, 0154, 0165, 0155, 0156, 0163, 0076, 0012, 0040, 
  0040, 0040, 0040, 0040, 0040, 0074, 0041, 0055, 0055, 0040, 0143, 0157, 0154, 0165, 0155, 0156, 
  0055, 0156, 0141, 0155, 0145, 0040, 0155, 0157, 0144, 0145, 0040, 0055, 0055, 0076, 0012, 0040, 
  0040, 0040, 0040, 0040, 0040, 0074, 0143, 0157, 0154, 0165, 0155, 0156, 0040, 0164, 0171, 0160, 
  0145, 0075, 0042, 0147, 0143, 0150, 0141, 0162, 0141, 0162, 0162, 0141, 0171, 0042, 0057, 0076, 
  0012, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0041, 0055, 0055, 0040, 0143, 0157, 0154, 0165, 
  0155, 0156, 0055, 0156, 0141, 0155, 0145, 0040, 0151, 0156, 0144, 0145, 0170, 0040, 0055, 0055, 
  0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0143, 0157, 0154, 0165, 0155, 0156, 0040, 
  0164, 0171, 0160, 0145, 0075, 0042, 0147, 0151, 0156, 0164, 0042, 0057, 0076, 0012, 0040, 0040, 
  0040, 0040, 0074, 0057, 0143, 0157, 0154, 0165, 0155, 0156, 0163, 0076, 0012, 0040, 0040, 0074, 
  0057, 0157, 0142, 0152, 0145, 0143, 0164, 0076, 0012, 0040, 0040, 0074, 0157, 0142, 0152, 0145, 
  0143, 0164, 0040, 0143, 0154, 0141, 0163, 0163, 0075, 0042, 0107, 0164, 0153, 0127, 0151, 0156, 
  0144, 0157, 0167, 0042, 0040, 0151, 0144, 0075, 0042, 0163, 0164, 0141, 0162, 0164, 0147, 0164, 
  0153, 0042, 0076, 0012, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 
  0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0143, 0141, 0156, 0137, 0146, 0157, 0143, 0165, 
  0163, 0042, 0076, 0106, 0141, 0154, 0163, 0145, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 
  0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 
  0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0164, 0151, 0164, 0154, 0145, 0042, 0040, 0164, 
  0162, 0141, 0156, 0163, 0154, 0141, 0164, 0141, 0142, 0154, 0145, 0075, 0042, 0171, 0145, 0163, 
  0042, 0076, 0112, 0106, 0104, 0165, 0153, 0145, 0063, 0104, 0074, 0057, 0160, 0162, 0157, 0160, 
  0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 
  0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0162, 0145, 0163, 0151, 0172, 0141, 
  0142, 0154, 0145, 0042, 0076, 0106, 0141, 0154, 0163, 0145, 0074, 0057, 0160, 0162, 0157, 0160, 
  0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 
  0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0167, 0151, 0156, 0144, 0157, 0167, 
  0137, 0160, 0157, 0163, 0151, 0164, 0151, 0157, 0156, 0042, 0076, 0143, 0145, 0156, 0164, 0145, 
  0162, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 
  0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 
  0042, 0164, 0171, 0160, 0145, 0137, 0150, 0151, 0156, 0164, 0042, 0076, 0144, 0151, 0141, 0154, 
  0157, 0147, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 
  0040, 0040, 0074, 0143, 0150, 0151, 0154, 0144, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 
  0074, 0160, 0154, 0141, 0143, 0145, 0150, 0157, 0154, 0144, 0145, 0162, 0057, 0076, 0012, 0040, 
  0040, 0040, 0040, 0074, 0057, 0143, 0150, 0151, 0154, 0144, 0076, 0012, 0040, 0040, 0040, 0040, 
  0074, 0143, 0150, 0151, 0154, 0144, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0157, 
  0142, 0152, 0145, 0143, 0164, 0040, 0143, 0154, 0141, 0163, 0163, 0075, 0042, 0107, 0164, 0153, 
  0102, 0157, 0170, 0042, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 
  0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0166, 0151, 
  0163, 0151, 0142, 0154, 0145, 0042, 0076, 0124, 0162, 0165, 0145, 0074, 0057, 0160, 0162, 0157, 
  0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 
  0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0143, 
  0141, 0156, 0137, 0146, 0157, 0143, 0165, 0163, 0042, 0076, 0106, 0141, 0154, 0163, 0145, 0074, 
  0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 
  0145, 0075, 0042, 0157, 0162, 0151, 0145, 0156, 0164, 0141, 0164, 0151, 0157, 0156, 0042, 0076, 
  0166, 0145, 0162, 0164, 0151, 0143, 0141, 0154, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 
  0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0143, 0150, 0151, 
  0154, 0144, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0157, 
  0142, 0152, 0145, 0143, 0164, 0040, 0143, 0154, 0141, 0163, 0163, 0075, 0042, 0107, 0164, 0153, 
  0102, 0157, 0170, 0042, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 
  0075, 0042, 0166, 0151, 0163, 0151, 0142, 0154, 0145, 0042, 0076, 0124, 0162, 0165, 0145, 0074, 
  0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 
  0040, 0156, 0141, 0155, 0145, 0075, 0042, 0143, 0141, 0156, 0137, 0146, 0157, 0143, 0165, 0163, 
  0042, 0076, 0106, 0141, 0154, 0163, 0145, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 
  0171, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 
  0143, 0150, 0151, 0154, 0144, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0074, 0157, 0142, 0152, 0145, 0143, 0164, 0040, 0143, 0154, 0141, 
  0163, 0163, 0075, 0042, 0107, 0164, 0153, 0102, 0157, 0170, 0042, 0076, 0012, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 
  0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0166, 0151, 0163, 
  0151, 0142, 0154, 0145, 0042, 0076, 0124, 0162, 0165, 0145, 0074, 0057, 0160, 0162, 0157, 0160, 
  0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 
  0156, 0141, 0155, 0145, 0075, 0042, 0143, 0141, 0156, 0137, 0146, 0157, 0143, 0165, 0163, 0042, 
  0076, 0106, 0141, 0154, 0163, 0145, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 
  0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 
  0075, 0042, 0155, 0141, 0162, 0147, 0151, 0156, 0137, 0164, 0157, 0160, 0042, 0076, 0061, 0062, 
  0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 
  0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0157, 0162, 0151, 0145, 
  0156, 0164, 0141, 0164, 0151, 0157, 0156, 0042, 0076, 0166, 0145, 0162, 0164, 0151, 0143, 0141, 
  0154, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0143, 0150, 
  0151, 0154, 0144, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0157, 0142, 0152, 0145, 0143, 0164, 0040, 0143, 
  0154, 0141, 0163, 0163, 0075, 0042, 0107, 0164, 0153, 0111, 0155, 0141, 0147, 0145, 0042, 0040, 
  0151, 0144, 0075, 0042, 0141, 0160, 0160, 0151, 0143, 0157, 0156, 0042, 0076, 0012, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 
  0075, 0042, 0167, 0151, 0144, 0164, 0150, 0137, 0162, 0145, 0161, 0165, 0145, 0163, 0164, 0042, 
  0076, 0066, 0064, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 
  0145, 0075, 0042, 0150, 0145, 0151, 0147, 0150, 0164, 0137, 0162, 0145, 0161, 0165, 0145, 0163, 
  0164, 0042, 0076, 0066, 0064, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 
  0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 
  0141, 0155, 0145, 0075, 0042, 0166, 0151, 0163, 0151, 0142, 0154, 0145, 0042, 0076, 0124, 0162, 
  0165, 0145, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 
  0075, 0042, 0143, 0141, 0156, 0137, 0146, 0157, 0143, 0165, 0163, 0042, 0076, 0106, 0141, 0154, 
  0163, 0145, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 
  0075, 0042, 0163, 0164, 0157, 0143, 0153, 0042, 0076, 0147, 0164, 0153, 0055, 0155, 0151, 0163, 
  0163, 0151, 0156, 0147, 0055, 0151, 0155, 0141, 0147, 0145, 0074, 0057, 0160, 0162, 0157, 0160, 
  0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 
  0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0151, 0143, 0157, 0156, 0137, 0163, 
  0151, 0172, 0145, 0042, 0076, 0060, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 
  0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0074, 0057, 0157, 0142, 0152, 0145, 0143, 0164, 0076, 0012, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0074, 0160, 0141, 0143, 0153, 0151, 0156, 0147, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 
  0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0145, 0170, 
  0160, 0141, 0156, 0144, 0042, 0076, 0106, 0141, 0154, 0163, 0145, 0074, 0057, 0160, 0162, 0157, 
  0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 
  0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0146, 0151, 0154, 0154, 0042, 
  0076, 0106, 0141, 0154, 0163, 0145, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 
  0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 
  0156, 0141, 0155, 0145, 0075, 0042, 0160, 0157, 0163, 0151, 0164, 0151, 0157, 0156, 0042, 0076, 
  0060, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 
  0057, 0160, 0141, 0143, 0153, 0151, 0156, 0147, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0057, 0143, 0150, 0151, 0154, 
  0144, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0074, 0143, 0150, 0151, 0154, 0144, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0157, 0142, 
  0152, 0145, 0143, 0164, 0040, 0143, 0154, 0141, 0163, 0163, 0075, 0042, 0107, 0164, 0153, 0114, 
  0141, 0142, 0145, 0154, 0042, 0040, 0151, 0144, 0075, 0042, 0141, 0160, 0160, 0164, 0151, 0164, 
  0154, 0145, 0042, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 
  0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0166, 0151, 0163, 0151, 0142, 0154, 0145, 
  0042, 0076, 0124, 0162, 0165, 0145, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 
  0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 
  0156, 0141, 0155, 0145, 0075, 0042, 0143, 0141, 0156, 0137, 0146, 0157, 0143, 0165, 0163, 0042, 
  0076, 0106, 0141, 0154, 0163, 0145, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 
  0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 
  0156, 0141, 0155, 0145, 0075, 0042, 0155, 0141, 0162, 0147, 0151, 0156, 0137, 0164, 0157, 0160, 
  0042, 0076, 0064, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 
  0145, 0075, 0042, 0154, 0141, 0142, 0145, 0154, 0042, 0040, 0164, 0162, 0141, 0156, 0163, 0154, 
  0141, 0164, 0141, 0142, 0154, 0145, 0075, 0042, 0171, 0145, 0163, 0042, 0076, 0112, 0106, 0104, 
  0165, 0153, 0145, 0063, 0104, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 
  0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0074, 0141, 0164, 0164, 0162, 0151, 0142, 0165, 0164, 0145, 0163, 
  0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0141, 0164, 0164, 0162, 0151, 0142, 0165, 
  0164, 0145, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0167, 0145, 0151, 0147, 0150, 0164, 0042, 
  0040, 0166, 0141, 0154, 0165, 0145, 0075, 0042, 0142, 0157, 0154, 0144, 0042, 0057, 0076, 0012, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0074, 0057, 0141, 0164, 0164, 0162, 0151, 0142, 0165, 0164, 0145, 0163, 
  0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0074, 0057, 0157, 0142, 0152, 0145, 0143, 0164, 0076, 0012, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0074, 0160, 0141, 0143, 0153, 0151, 0156, 0147, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 
  0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0145, 0170, 
  0160, 0141, 0156, 0144, 0042, 0076, 0106, 0141, 0154, 0163, 0145, 0074, 0057, 0160, 0162, 0157, 
  0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 
  0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0146, 0151, 0154, 0154, 0042, 
  0076, 0124, 0162, 0165, 0145, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 
  0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 
  0141, 0155, 0145, 0075, 0042, 0160, 0157, 0163, 0151, 0164, 0151, 0157, 0156, 0042, 0076, 0061, 
  0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0057, 
  0160, 0141, 0143, 0153, 0151, 0156, 0147, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0057, 0143, 0150, 0151, 0154, 0144, 
  0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0074, 0057, 0157, 0142, 0152, 0145, 0143, 0164, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0141, 0143, 0153, 0151, 0156, 0147, 
  0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 
  0075, 0042, 0145, 0170, 0160, 0141, 0156, 0144, 0042, 0076, 0106, 0141, 0154, 0163, 0145, 0074, 
  0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 
  0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0146, 0151, 0154, 0154, 0042, 
  0076, 0106, 0141, 0154, 0163, 0145, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 
  0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 0141, 0155, 0145, 
  0075, 0042, 0160, 0141, 0144, 0144, 0151, 0156, 0147, 0042, 0076, 0061, 0062, 0074, 0057, 0160, 
  0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 
  0164, 0171, 0040, 0156, 0141, 0155, 0145, 0075, 0042, 0160, 0157, 0163, 0151, 0164, 0151, 0157, 
  0156, 0042, 0076, 0060, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0076, 0012, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0057, 
  0160, 0141, 0143, 0153, 0151, 0156, 0147, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0074, 0057, 0143, 0150, 0151, 0154, 0144, 0076, 0012, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0143, 0150, 0151, 0154, 0144, 
  0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0074, 0157, 0142, 0152, 0145, 0143, 0164, 0040, 0143, 0154, 0141, 0163, 0163, 0075, 0042, 0107, 
  0164, 0153, 0116, 0157, 0164, 0145, 0142, 0157, 0157, 0153, 0042, 0040, 0151, 0144, 0075, 0042, 
  0164, 0141, 0142, 0163, 0042, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 
  0040, 0156, 0141, 0155, 0145, 0075, 0042, 0167, 0151, 0144, 0164, 0150, 0137, 0162, 0145, 0161, 
  0165, 0145, 0163, 0164, 0042, 0076, 0064, 0065, 0060, 0074, 0057, 0160, 0162, 0157, 0160, 0145, 
  0162, 0164, 0171, 0076, 0012, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 0040, 
  0040, 0040, 0040, 0040, 0040, 0074, 0160, 0162, 0157, 0160, 0145, 0162, 0164, 0171, 0040, 0156, 
  01