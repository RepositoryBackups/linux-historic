#section: printers
#format: make\tmodel\tcolor\tfunc\ttype_verbose\tgs_driver\tlang_verbose\tx\ty\tgs_driver_url\t
ascii\tpnp_mfg\tpnp_mdl\tpnp_cmd\tpnp_des\tnotes
Alps    MD-1000 Color   D       Thermal transfer        md2k    Proprietary     1200    600
http://plaza26.mbn.or.jp/~higamasa/gdevmd2k/    F                                       Appears
 to operate to full spec with this Ghostscript driver, but as the documentation is in Japanese 
I'm not sure.
Alps    MD-1300 Color   D       Thermal transfer        md2k    Proprietary     1200    600
http://plaza26.mbn.or.jp/~higamasa/gdevmd2k/    F                                       Also su
pports dye sublimation\n<br>Appears to be supported by md2k Ghostscript driver, but since docum
entation is in Japanese I'm a little fuzzy on the details.
Alps    MD-2000 Color   D       Thermal transfer        md2k    Proprietary             
http://plaza26.mbn.or.jp/~higamasa/gdevmd2k/    F                                       Listed 
as supported in the driver documentation.
Alps    MD-4000 Color   D       Dye Sublimation md2k    Proprietary                     http://
plaza26.mbn.or.jp/~higamasa/gdevmd2k/   F                                       Listed as suppo
rted at 300/600/12x6 dpi in the driver documentation.\nI have not found any other reference to 
this printer.
Alps    MD-5000 Color   D       Thermal transfer        md5k    Proprietary     2400    2400
http://plaza26.mbn.or.jp/~higamasa/gdevmd2k/    F                                       Dye sub
limation option available.\n<br>This driver appears to support 1200x600, 600, and 300dpi operat
ion on a variety of papers.\nUnfortunately it is documented in Japanese.
Apple   12/640ps        BW      B       Laser   N/A     PostScript      600     600     N/A
T                                       No specific Linux driver available\nworks fine with Gen
eric Postscript\nDriver from parallel port, not sure\nif it works from the ethernet.
Apple   Dot Matrix      BW      A       Dot Matrix      appledmp        Proprietary     120
72      http://www.cs.wisc.edu/~ghost/printer.html      T                               

Apple   ImageWriter LQ  BW      A       Dot Matrix      iwlq    Proprietary     320     216
http://www.cs.wisc.edu/~ghost/printer.html      T                                       
Apple   ImageWriter high-res    BW      A       Dot Matrix      iwhi    Proprietary     120
144     http://www.cs.wisc.edu/~ghost/printer.html      T                               

Apple   ImageWriter low-res     BW      A       Dot Matrix      iwlo    Proprietary     120
72      http://www.cs.wisc.edu/~ghost/printer.html      T                               

Apple   LaserWriter 16/600      BW      A       Laser   N/A     PostScript      600     600
N/A     T                                       
Apple   LaserWriter IINTX       BW      A       Laser   N/A     PostScript      300     300
N/A     F                                       Set Switch 1 and 2 down, all others up, use ser
ial connection at 9600 baud with XON/XOFF handshaking.  Send the printer postscript.  Apple's w
eb pages have good information on switch settings, and even a suggested printcap file.  Search 
for "LaserWriter AND unix" in thier Older Hardware section.
Apple   LaserWriter Select 360  BW      A       Laser   ljet2p  PostScript      300     300
http://www.cs.wisc.edu/~ghost/printer.html      T                                       No luck
 with any apple drivers, however\nI selected HPLaserJet IId/IIp/III with\nTIFF Compression in P
rinttool.  Work great.
Apple   StyleWriter 2500        Color   B       Ink Jet N/A     Proprietary     360     360
N/A     T                                       Color is a bit dark, but all else \nworks.  Pap
er out and jammed errors\nare handled better than MacOS!
Avery   Personal Label Printer  BW      B       Thermal pbm2lwxl        Proprietary     128
128     http://www.freelabs.com/~whitis/software/pbm2lwxl/      F                       
        OEM LabelWriter?<br>1.5 inch wide label printer; driver by Mark Whitis<br>Known to work
 with minor glitches; see driver's web page.
Avery   Personal Label Printer+ BW      A       Thermal pbm2lwxl        Proprietary     192
192     http://www.freelabs.com/~whitis/software/pbm2lwxl/      F                       
        OEM LabelWriter?<br>2.25 inch label printer; driver by Mark Whitis<br>Should work; unte
sted
Brother HJ-400  BW      B       Ink Jet lq850   ESC/P   360     360     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       The lq850 driver doesn't come w
ith RedHat by default.\n<br>The epson driver can work at 180x180, but looks bad at 180x360 and 
360x360\n<br>The printer can also emulate an IBM PS/2 according the the manuals, but I've never
 got this to work, even under windows.\n<br>Draft mode has to be set using the switches on the 
front.\n<p>For TeX, use the nechi driver. Its the same as the LQ, but at 360x360.
Brother HL-1030 BW      F       Laser   N/A     Proprietary     600     600     N/A     F
                                10 ppm. Excellent quality under Windows, but no Linux driver ye
t.\nUses GDI-driver under Windows. No PCL, no ASCII.\nSuccessor of the HL-820, but does not wor
k with "hl7x0" driver.
Brother HL-1040 BW      B       Laser   ljet2p  PCL     600     600     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       Limited to 300dpi with Ghostscr
ipt.\nPrinting slower under GS than under Windows.\nToner good for 2400 pages @ 5%, Drum life @
\n20K pages. Only limitatons noted compared\nto Windows are resolution and speed.\nOtherwise, e
verything seems to work fine.\nDOS driver for emulated PCL-5, may work\nunder DOS-EMU.
Brother HL-1050 BW      B       Laser   ljet4   PCL     1200    600     http://www.cs.wisc.edu/
~ghost/printer.html     T       Brother HL-1050 series  PCL5,PJL,PCLXL          10 ppm. Excelle
nt quality printing at 600x600 DPI.\n1200x600dpi not available without proprietary driver.\nExc
ellent paper handling, fast printing from Linux/Ghostscript.  \nUSB interface included.  Expand
able to 36MB with conventional 72-pin DRAM.
Brother HL-1060 BW      B       Laser   ljet4   PCL     1200    600     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       Works perfectly at 600x600dpi. 
 1200x600 requires a special driver.  With optional BR-Script2, Postscript2 emulation seems per
fect, but slow, at 600x600dpi.
Brother HL-1070 BW      A       Laser   N/A     PostScript      1200    600     N/A     T
Brother HL-1070 series  PCL5,POSTSCRIPT2,PJL,PCLXL              This printer works flawlessly; 
just plug it\nin and go.
Brother HL-10V  BW      A       Laser   ljet3   PCL     300     300     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       HPII or HPIII drivers work. \nS
upports HP-GL/2, IBM Proprinter \nXL Mode, Epson FX-85 & Brother \nTwinriter DP modes, & Diablo
 630 \nand Brother WP modes.
Brother HL-10h  BW      A       Laser   ljet4   PCL     600     600     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       
Brother HL-1240 BW      F       Laser   N/A     Proprietary     600     600     N/A     F
                                PCL support only under Windows.\n<br>Appears to be a Winprinter
 with Mac drivers, but no Linux drivers.
Brother HL-1250 BW      B       Laser   ljet4   PCL     1200    600     http://www.cs.wisc.edu/
~ghost/printer.html     T       Brother HL-1250 series  PCL5,PJL,PCLXL          12 ppm. Excelle
nt quality printing at 600x600 DPI. 1200x600dpi not available without proprietary driver. Excel
lent paper handling, fast printing from Linux/Ghostscript. USB interface included. Expandable t
o 36MB with conventional 72-pin DRAM.
Brother HL-1260 BW      A       Laser   N/A     PostScript      600     600     N/A     T
        Brother HL-1260 PCL5,POSTSCRIPT2,PJL            Can also use HP LaserJet 4 driver. 2MB 
standard, expandable to 26MB with standard 72 pin SIMMS.
Brother HL-2060 BW      A       Laser   N/A     PostScript      1200    1200    N/A     T
                                8MB memory std; note that this isn't enough for 1200dpi.\n<br>F
ujitsu SPARClite based.\n<br>Network option available, USB, Parellel, Serial std.\n<br>Also spe
aks PCL 6, HPGL, Epson.\n<br>Duplex option available.
Brother HL-4Ve  BW      A       Laser   laserjet        PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
Brother HL-630  BW      A       Laser   ljet2p  PCL     300     300     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       Reported as working perfectly b
y gjames.\n<br>0.5MB memory std; may need more to print beyond 150dpi.\n<br>The model 631 appea
rs to be the same, but with 1.0M RAM.
Brother HL-660  BW      A       Laser   ljet4   PCL     600     600     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       Postscript option.
Brother HL-720  BW      A       Laser   hl7x0   Proprietary     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     F               Brother HL-720 series                   Driver 
appears in gs distributions > 5.10.\n<br>Gs will drive it at 6ppm.
Brother HL-730  BW      A       Laser   hl7x0   PCL     600     600     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       Listed as working in the Ghosts
cript sources.
Brother HL-760  BW      A       Laser   ljet4   PCL     1200    600     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       1MB standard; may require more 
for 600 and 300dpi.
Brother HL-8    BW      A       Laser   ljetplus        PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Essentially a LaserJet 
II with 512K RAM.  I put in an extra\n2MB module so I could print full-page 300dpi, otherwise y
ou\ncan only do 150dpi.
Brother HL-820  BW      B       Laser   hl7x0   Proprietary     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     F                                       Documented by Brother a
s a Windows printer.\n<br>2 MB Memory not expandable\n<br>works with Ghostscript hl7x0 driver a
t 600 DPI
Brother MC-3000 Color   D       Ink Jet epson   ESC/P   720     720     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       This is a GDI (winprinter) prin
ter/scanner/copier, with ESC/P support<br>\nCould only get it working under Unix with esc/p (th
e Ghostscript epson driver),\n- output is in b/w 360x180dpi :(<br>\nScanning won't work at all.
Brother MFC 6550MC      BW      B       Laser   ljet2p  PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Prints Perfectly, just 
no scanning support.\n<br>Use GS 5.50. Slow under StarOffice 5.1. Works fine under WordPerfect 
8 for Linux.
Brother MFC 7150C       Color   D       Ink Jet epsonc  Proprietary     1440    720     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Prints in black
 and white at \nI don't know what resolution.\n<br>\nScanning presumed unsupported?
CalComp Artisan 1023 penplotter Color   F       Impact  N/A     PCL                     N/A
F                                       
Canon   BJ-10e  BW      A       Ink Jet bj10e   Proprietary                     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Note that some report i
t working properly in epson mode only.
Canon   BJ-20   BW      A       Ink Jet bj10e   Proprietary                     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       works with ghostscript 
as of v 5.10.
Canon   BJ-200  BW      A       Ink Jet bj200   Proprietary     360     360     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
Canon   BJ-300  BW      B       Ink Jet lq850   ESC/P                   http://www.cs.wisc.edu/
~ghost/printer.html     T                                       Drives Canon at 360dpi in Epson
 emulation mode.
Canon   BJ-330  BW      A       Ink Jet bj200   ESC/P   360     360     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       Drives Canon at 360dpi in Epson
 emulation mode.
Canon   BJ-5    BW      A       Ink Jet bj10e   Proprietary     360     360     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Used APSfilter to gener
ate printcap
Canon   BJC-1000        Color   B       Ink Jet bjc600  Proprietary     360     720     http://
www.cs.wisc.edu/~ghost/printer.html     F       Canon   BJC-1000        BJL,BJRaster,BSCC,TXT01
Canon BJC-1000  This printer works fine with the Ghostscript driver for the BJC-600/BJC-4000. H
owever, it will not do plain ASCII text.
Canon   BJC-2000        Color   B       Ink Jet bjc600  Proprietary     720     360     http://
www.cs.wisc.edu/~ghost/printer.html     F       Canon   BJC-2000        BJL,BJRaster,BSCC,TXT01
Canon BJC-2000  Won't do 720x360 mode, AFAIK; You only get 1/2 of a stretched image.\n\nThis pr
inter claims to use a PPA interface, but it must (also?) work with the bjc600 protocol.
Canon   BJC-210 Color   A       Ink Jet bjc600  ESC/P   720     360     http://www.cs.wisc.edu/
~ghost/printer.html     T       Canon   BJC-210 BJ,LQ,BJL,BJRaster      Canon BJC-210   Use bj2
00 gs driver for bw.  \n<br>Colour printing is only at 360x360.\n<br>For TeX, it may be\nnecess
ary to set a 360x360 or 720x720 metafont mode for some fonts which don't transform correctly at
\n720x360.
Canon   BJC-240 Color   B       Ink Jet bjc600  ESC/P   360     360     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       use bj200 gs driver for bw.
Canon   BJC-250 Color   B       Ink Jet bj200   ASCII   360     360     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       Use bj200 driver for bw.\n<br>U
se uniprint parameter files bjc610*.upp for color.
Canon   BJC-4000        Color   A       Ink Jet bjc600  ESC/P   360     360     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Use gs option -sProcess
ColorModel=DeviceGray for bw.\n<br>May need proprietary software to load, but there seems to be
 a funky button combination that works, too.
Canon   BJC-4100        Color   A       Ink Jet bjc600  ESC/P   720     360     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Use gs option -sProcess
ColorModel=DeviceGray for bw.
Canon   BJC-4200        Color   A       Ink Jet bjc600  ESC/P   720     360     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Direct ASCII print to t
he parallel\nport don't work (CR are not taken\ninto acount by the printer, so that\nlines are 
not wrapped)
Canon   BJC-4300        Color   A       Ink Jet bjc600  ESC/P   720     360     http://www.cs.w
isc.edu/~ghost/printer.html     T       Canon   BJC-4300        BJ,LQ,BJL,BJRaster,BSCC Canon B
JC-4300 Cartridge loading and alignment can be done from front panel; consult your manual.\n<br
>May require a recent (5+) of ghostscript.
Canon   BJC-4310SP      Color   B       Ink Jet bjc600  ESC/P   360     360     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Colour doesn't work for
 less than 24 bits/pixel.
Canon   BJC-4400        Color   A       Ink Jet bjc600  ESC/P   720     360     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
Canon   BJC-4550        Color   D       Ink Jet bjc600  Proprietary                     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       In gs 5.03, col
our only works for 24 and 32\nbit/pixel. \n<br>Monochrome reported not working, but try bj200 d
river or -dBitsPerPixel=1. \n<br>Maximum width is limited to 8.5 inches.
Canon   BJC-5000        Color   F       Ink Jet N/A     Proprietary     1440    720     N/A
T                                       Dan Herrglock (?) reports on Usenet that this printer w
orks "fine" with the bjc600 driver.\n<br>All other reports say that it's a paperweight.
Canon   BJC-5100        Color   F       Ink Jet N/A     PPA     1440    720     N/A     T
                                If someone believes this printer works -clearly document how to
 get it to work HERE.  It simply does not work since the protocol is proprietary and undisclose
d.
Canon   BJC-600 Color   A       Ink Jet bjc600  Proprietary     360     360     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Specific GS driver writ
ten for this printer.
Canon   BJC-6000        Color   A       Ink Jet bjc600  PPA     1440    720     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Reported to work on Use
net by Jan Harmsen; may well be "perfect" - if so please fix!\n<br>May only run at 720x720?
Canon   BJC-610 Color   A       Ink Jet uniprint        Proprietary     720     720     http://
www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint T                               
Uses bjc610*.upp files; see uniprint driver link.\n<br>Different uniprint drivers for 360/720dp
i printing and different paper types.
Canon   BJC-620 Color   A       Ink Jet bjc600  Proprietary     720     720     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Uses alternately bjc610
*.upp files; see uniprint driver link.\nDifferent uniprint drivers for 360/720dpi printing and 
different paper types.
Canon   BJC-70  Color   A       Ink Jet bjc600  Proprietary                     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       use bj20 gs driver for 
bw.
Canon   BJC-7000        Color   D       Ink Jet bjc800  ESC/P   1200    600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       works in color.  bw?
Canon   BJC-7004        Color   B       Ink Jet bjc800  Proprietary     1200    600     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Works at 600x60
0.
Canon   BJC-7100        Color   D       Ink Jet bjc800  Proprietary     1200    600     http://
www.cs.wisc.edu/~ghost/printer.html     F                                       very slow\n<br>
CMYKcmy 7-color printer; current drivers are CMYK/4-color only.
Canon   BJC-80  Color   B       Ink Jet bjc600  Proprietary     720     360     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Portable printer.\n<br>
Serial(IrDA) or parallel interfaces; both work.\n<br>Scanner functionality is not supported.\n<
br>See the more info URL for information on configuring lpd to use a serial port properly for t
his printer.
Canon   BJC-800 Color   A       Ink Jet bjc800  Proprietary     720     720     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
Canon   LBP-1260        BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Works fine with 7 MB in
stalled
Canon   LBP-1760        BW      A       Laser   ljet4   PCL     1200    1200    http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Works fine, although I 
have only tried it up to 600x600 on A4
Canon   LBP-4+  BW      A       Laser   lbp8    Proprietary                     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Reported on Usenet by M
ark Bennett-Leaver to work fine with the lbp8 driver.\n<br>Note that pages with lots of graphic
s will require additional printer memory.
Canon   LBP-430 BW      F       Laser   N/A     PPA     300     300     N/A     F       
                        
Canon   LBP-460 BW      F       Laser   N/A     Proprietary     300     300     N/A     F
                                PCL4/LJIIp emulation via Windows driver.
Canon   LBP-4U  BW      A       Laser   lbp8    Proprietary     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Text and graphics work 
well with Ghostscript's lbp8 driver (but the page is shifted). It is more convenient to use the
 ljetIII driver then convert the PCL output to CaPSL with cjet 0.89.
Canon   LBP-4sx BW      B       Laser   ljet3   PCL     300     300     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       Reported to work fine as a PCL 
laser by Alan Dickie.
Canon   LBP-660 BW      F       Laser   N/A     Proprietary     600     600     N/A     F
                                Offers PCL emulation, but this appears to be in software.\n<br>
Even if PCL is in firmware, it is limited to 300dpi.
Canon   LBP-8A1 BW      A       Laser   N/A     PPA                     N/A     F       
                        
Canon   LBP-8II BW      A       Laser   lbp8    Proprietary                     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
Canon   LIPS III        BW      A       Laser   lips3   Proprietary     300     300     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
Canon   LIPS-III        BW      A       Laser   lips3   Proprietary                     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Gs driver works
 in English mode (?).
Canon   MultiPASS C2500 Color   D       Ink Jet bjc600  Proprietary                     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       printer/fax/cop
ier\n<br>works in 360x360\n<br>You have to push the Printer button.
Canon   MultiPASS C3500 Color   D       Ink Jet bjc600  Proprietary     720     360     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Using GhostScri
pt 5.1, can get\nreasonable 360x360 output using the\n'bjc600' driver.  Doesn't support\nthe fa
xing/scanning though (although\nI haven't tried SANE yet).  Note\nthat you do have to configure
 the\nprinter to run in 'DOS emulation'\nmode in order to get it to print\nproperly when using 
this driver.
Canon   MultiPASS C5000 Color   D       Ink Jet bjc600  Proprietary     720     360     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Can be set to D
OS emulation: information is not in the manual but is available from the CCSI web site. Still c
an't do much more than print ASCII, however.
Canon   Multipass C3000 Color   D       Ink Jet bjc600  Proprietary                     http://
www.cs.wisc.edu/~ghost/printer.html     F                                       Actually, I use
d the bjc610 uniprint driver\nsetting from RedHat's printtool.\nThis multifunction machine igno
red me until\nI gave it this magic invocation from \nthe front panel:\n"Function" "0" five-grea
ter-than-signs "Start" "Start"\n(Which corresponds to DOS Printing ON)
Canon   Multipass C5500 Color   D       Ink Jet bjc600  Proprietary     720     360     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       This is a multi
function printer that differs from the C3500 only in the fact that the scanner is a color one (
in the C3500 it is B&W).\n\nThe documentation that comes with the printer claims that this will
 not run with NT or Win 3.1. However, the canon site has links to drivers for NT.\nPrinting und
er non-Win9X systems is accomplished by setting the printer to DOS emulation mode. Under this m
ode, the printer is driver compatible (according\nto Canon) with several printers including the
 Epson LQ2550 and LQ2500 and the Canon BJC 4300 and 4400. The resolution in this case is claime
d to be the \nmaximum supported by the driver.\n\nI tried it out with the bj600 and it works. R
esolution at which it prints is not known. I have not tried the scanner.
Canon   Multipass L6000 BW      F       Laser   N/A     Proprietary     600     600     N/A
F                                       Multifunction device (fax/copier/etc).\n<br>Vendor FAQ 
says no support for DOS apps at all.\n<br>Matt Berger tried various drivers with no luck.
Citizen ProJet II       BW      A       Ink Jet laserjet        PCL     300     300     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Clone of Olivet
ti JP350, hence similarly functional.
Citizen ProJet IIc      Color   A       Ink Jet djet500 PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Clone of Olivetti JP450
.
Citizen printiva600C    Color   D       Thermal N/A     PCL     600     600     N/A     T
                                I have a fully functional\ndriver from PPM files.\nStarted hack
ing ghostscript,\nbut abandoned it.\nContact me if you want ppmtocpva.\njcb@dcs.ed.ac.uk
DEC     1800    BW      D       Laser   ljet3   ESC/P   300     300     http://www.cs.wisc.edu/
~ghost/printer.html     F                                       appears to be same as Acer 506G
11 also Citizen ProLaser 600, Minolta SP-6, Epson 5200, only partial function - ok for text/ASC
II, some graphics, but HP PJL not supported. still resolving - watch this space.\n\nIs a 3rd pa
rty product which is OEM badged, even by Epson ! (who don't make it)
DEC     DECWriter 500i  BW      A       Ink Jet djet500 PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       A DeskJet 500 clone bui
lt by Olivetti.
DEC     DECwriter 110i  BW      A       Ink Jet djet500 PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Set up like a black-and
-white Deskjet.  Might need to\nincrease bottom margin slightly.
DEC     DECwriter 520ic Color   A       Ink Jet cdj500  PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Use cdj500 for color; d
j500 for B/W.  Easiest way is to set up two queues, one\ncolor, the other B/W.  Colors are a li
ttle muddy, but\notherwise perfect.
DEC     LA50    BW      A       Dot Matrix      la50    Proprietary     144     72      http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
DEC     LA75    BW      A       Dot Matrix      la75    Proprietary     144     72      http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
DEC     LA75 Plus       BW      A       Dot Matrix      la75plus        Proprietary     180
180     http://www.cs.wisc.edu/~ghost/printer.html      T                               

DEC     LJ250   BW      B       Ink Jet declj250        Proprietary                     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
DEC     LN03    BW      A       Laser   ln03    Proprietary     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Serial Port Printer \nR
equires\n        :lp=/dev/cua0:\n        :br#9600:\nin printcap\n\n178,000 pages and still perf
ect.
Dymo-CoStar     ASCII 250       BW      A       Thermal pbm2lwxl        Proprietary     203
203     http://www.freelabs.com/~whitis/software/pbm2lwxl/      T                       
        1.5 inch wide label printer; driver by Mark Whitis<br>Should work; untested
Dymo-CoStar     ASCII+  BW      A       Thermal pbm2lwxl        Proprietary     192     192
http://www.freelabs.com/~whitis/software/pbm2lwxl/      T                               
2.25 inch wide label printer; driver by Mark Whitis<br>Should work; untested
Dymo-CoStar     EL40    BW      A       Thermal pbm2lwxl        Proprietary     192     192
http://www.freelabs.com/~whitis/software/pbm2lwxl/      F                               
1.5 inch wide label printer; driver by Mark Whitis<br>Should work fine; untested.
Dymo-CoStar     EL60    BW      A       Thermal pbm2lwxl        Proprietary     192     192
http://www.freelabs.com/~whitis/software/pbm2lwxl/      F                               
2.25 inch wide label printer; driver by Mark Whitis<br>Should work; untested
Dymo-CoStar     LabelWriter II  BW      A       Thermal pbm2lwxl        Proprietary     192
192     http://www.freelabs.com/~whitis/software/pbm2lwxl/      F                       
        1.5 inch wide label printer; driver by Mark Whitis<br>Should work fine; untested.<br>Ma
ybe only 128dpi?
Dymo-CoStar     LabelWriter XL  BW      A       Thermal pbm2lwxl        Proprietary     192
192     http://www.freelabs.com/~whitis/software/pbm2lwxl/      F                       
        1.5 inch wide label printer; driver by Mark Whitis<br>Known to work.
Dymo-CoStar     LabelWriter XL+ BW      A       Thermal pbm2lwxl        Proprietary     192
192     http://www.freelabs.com/~whitis/software/pbm2lwxl/      F                       
        2.25 inch wide label printer; driver by Mark Whitis<br>Should work fine; untested.
Dymo-CoStar     SE250   BW      A       Thermal pbm2lwxl        Proprietary     203     203
http://www.freelabs.com/~whitis/software/pbm2lwxl/      F                               
2.25 inch label printer; driver by Mark Whitis<br>Should work; untested
Dymo-CoStar     SE250+  BW      A       Thermal pbm2lwxl        Proprietary     192     192
http://www.freelabs.com/~whitis/software/pbm2lwxl/      F                               
2.25 inch wide label printer; driver by Mark Whitis<br>Should work; untested
Dymo-CoStar     Turbo   BW      A       Thermal pbm2lwxl        Proprietary     192     192
http://www.freelabs.com/~whitis/software/pbm2lwxl/      F                               
2.25 inch wide label printer; driver by Mark Whitis<br>Should work fine; untested
Epson   9 Pin Printers high-res BW      A       Dot Matrix      eps9high        Proprietary
240     216     http://www.cs.wisc.edu/~ghost/printer.html      T                       
        
Epson   9 Pin Printers med-res  BW      A       Dot Matrix      eps9mid Proprietary     240
216     http://www.cs.wisc.edu/~ghost/printer.html      T                               

Epson   AP3250  BW      A       Ink Jet ap3250  ESC/P   360     360     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       Just copying over entries from 
the old Red Hat printtool database (we're moving over to using this one) - not sure whether or 
not this is a color printer.
Epson   ActionLaser 1100        BW      A       Laser   ljet3   ESC/P                   http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
Epson   EPL 5700        BW      B       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T       EPSON   EPL-5700        PJL,EJL,ESCPL2-00,ESCP9-84,PRPX
L24-01,HP ENHANCED PCL5,HPGL2-01,       EPSON EPL-5700  Works OK but only in 300 dpi mode. 600 
dpi is mostly OK but some glitches appear in text.
Epson   LP 8000 BW      A       Laser   lp8000  Proprietary                     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
Epson   LQ 850  BW      A       Dot Matrix      lq850   ESC/P                   http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Gs driver works at 360d
pi.
Epson   LQ-24   Color   A       Dot Matrix      epsonc  Proprietary     240     216     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
Epson   LQ-2550 Color   A       Dot Matrix      epsonc  Proprietary     240     72      http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
Epson   LQ-570+ BW      A       Dot Matrix      N/A     ESC/P                   N/A     T
                                The EPSON User's Guide couples the LQ-570+ with the LQ-1070+ on
 the "configuration menu". How the printer performs under UNIX I do not know because I am waiti
ng for the UNIX CD to be delivered to me.
Epson   LX-1050 BW      A       Dot Matrix      N/A     PPA                     N/A     F
                                
Epson   SQ 1170 BW      A       Ink Jet stcolor ESC/P   360     360     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       
Epson   Stylus 300      BW      B       Ink Jet st800   ESC/P   360     360     http://www.cs.w
isc.edu/~ghost/printer.html     F                                       TRy using the drivers f
or the Stylus 800.
Epson   Stylus Color    Color   A       Ink Jet uniprint        ESC/P   720     720     http://
www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint T                               
Uniprint parameter files stc.upp, stc_l.upp, or stc_h.upp.\n<br>CMYK 4 color printing.\n<br>Sho
uld also work with stcolor gs driver.
Epson   Stylus Color 1520       Color   A       Ink Jet uniprint        ESC/P   1440    720
http://www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint  T       EPSON   Stylus COLOR 15
20      ESCP2E,PRPXL            Will also work with stcolor driver (non-uniprint) at 360x360 DP
I.\nUniPrint driver stc1520h
Epson   Stylus Color 300        Color   D       Ink Jet uniprint        ESC/P   360     360
http://www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint  F                       
        Works partially with the ghostscript 5.10 stcany.upp and stc2.upp \ndrivers and -sDEVIC
E=stcolor -dMicroweave. \nThe printout is readable but not good.\n<br>Glenn Ramsey, Usenet cont
ributor, is working on uniprint files.
Epson   Stylus Color 3000       Color   B       Ink Jet stcolor ESC/P   1440    720     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Using the Epson
 Stylus Color ESP/2 \ndrivers, max resolution is 360dpi.\nMost other features are working, \nth
ough. Epson sends a RIP module \nwith the windows drivers that is a\nPS to ESP/2 converter. I b
ought the\nprinter expecting it to have native\nPS but it doesn't.
Epson   Stylus Color 400        Color   A       Ink Jet stcolor ESC/P   720     720     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Works best with
 stcolor driver using the -dSoftweave parameter. Also works with uniprint driver and stc500p.up
p and stc500ph.upp parameter files, but printing at 720 dpi is VERY slow this way. Implements a
 subset of ESC/P2 language. Third-party ink cartridges and refill kits available.
Epson   Stylus Color 440        Color   A       Ink Jet stcolor ESC/P   720     720     http://
www.cs.wisc.edu/~ghost/printer.html     T       EPSON   Stylus COLOR 440        ESCPL2,BDC
EPSON Stylus COLOR 440  Redhat printtool:EXTRA_GS_OPTIONS="stcolor"\n<br>\nI use 360x360 PS res
olution and it is very nice.\n<br>\n21 jets per color.  stcolor drives a different number of je
ts; you can \nuse the uniprint driver stc.upp instead if you change upOutputPins to 21.
Epson   Stylus Color 500        Color   A       Ink Jet uniprint        ESC/P   720     720
http://www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint  T                       
        Uniprint files:\nstc500p.upp = 360x360\nstc500ph.upp = 720x720
Epson   Stylus Color 600        Color   A       Ink Jet uniprint        ESC/P   1440    720
http://www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint  T                       
        Uniprint files:\nstc600ih.upp = 1440x720\nstc600p.upp = 720x720\nstc600pl.upp = 360x360
Epson   Stylus Color 640        Color   A       Ink Jet stcolor ESC/P   1440    720     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Only works at 3
60x360 with stcolor and ghostscript 4.03. \n<br>Redhat users (and probably others) can upgrade 
to ghostscript 5.50, printtool 3.40 and rhs-printfilters 1.50 and use the Epson Stylus Color mo
des stc600ih, stc600p and st600pl (1440, 720 and 360 dpi).\n<br>4 color CMYK printing with 32 j
ets per color.
Epson   Stylus Color 660        Color   A       Ink Jet uniprint        ESC/P   1440    720
http://www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint  T                       
        <p>The uniprint subdriver is \n@stc600ih.upp. </p>\n<br>\n<p>I have only tested at 1440
x720. \nPrinting is extremely slow at that\nresolution, but text quality is \nexcellent.  Image
s seem to be a \nbit posterized. I haven't played \nwith any (e.g. ghostscript) \nsettings to s
ee if this can be \nfixed. </p>
Epson   Stylus Color 740        Color   B       Ink Jet uniprint        ESC/P   720     1440
http://www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint  T       EPSON   Stylus COLOR 74
0       ESCPL2,BDC,D4   EPSON Stylus COLOR 740  DEPontius at usa.net is working on upp file for
 700 series; find them at the More Info link.\nAt the moment all resolutions work properly, and
 only a bit of color tuning is missing.\n<br>Uniprint parameter files stc740*.upp.\n<br>Philip 
Dodd also reports excellent photo results with the stc2_h.upp file.\n<br>4 color CMYK printing 
with 48 jets per color.\n<br>Obviusly also "transparent blue" version works the same way
Epson   Stylus Color 800        Color   A       Ink Jet uniprint        ESC/P   1440    720
http://www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint  T       EPSON   Stylus COLOR 80
0       ESCPL2,PRPXL24,BDC              Use the stc800 uniprint files.
Epson   Stylus Color 850        Color   A       Ink Jet uniprint        ESC/P   1440    720
http://www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint  T                       
        used stc800ih.upp for making some color overheads, including\nsome molecular model pict
ures.\n<br>4 color CMYK printing with 64 jets per color.
Epson   Stylus Color 900        Color   D       Ink Jet uniprint        ESC/P   1440    720
http://www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint  F                       
        No specific Uniprint parameter file has yet been written, but stcany.upp and stc500*.up
p should work.\n<br>Please pipe up if you've written a upp file for this printer!\n<br>GIMP's S
tylus Color 800 driver works.
Epson   Stylus Color I  Color   A       Ink Jet uniprint        ESC/P   720     720     http://
www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint T                               
Uniprint files:\nstc.upp = 360x360\nstc_h.upp = 720x720\nstc_l.upp = 360x360 noWeave
Epson   Stylus Color II Color   A       Ink Jet uniprint        ESC/P   720     720     http://
www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint T                               
Uniprint parameter files stc2.upp, stc2_h.upp.\n<br>CMY 3-color OR Black-only printing; the bla
ck ink can't mix!
Epson   Stylus Color IIs        Color   A       Ink Jet uniprint        ESC/P   720     720
http://www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint  T                       
        Uniprint parameter files stc2.upp, stc2s_h.upp.\n<br>3 color CMY printing, or black wit
h separate cartridge.
Epson   Stylus Color PRO        Color   A       Ink Jet uniprint        ESC/P   720     720
http://www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint  T                       
        Uniprint files:\nstc.upp = 360x360\nstc_h.upp = 720x720\nstc_l.upp = 360x360 noWeave
Epson   Stylus Photo 700        Color   D       Ink Jet uniprint        ESC/P   1440    720
http://www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint  T                       
        6 color CMYKcm printing with 32 jets per color.\n<br>Probably works OK, in 4 colors, wi
th 600 upp files; see Stylus Color 640 entry.
Epson   Stylus Photo 750        Color   D       Ink Jet uniprint        ESC/P   1440    720
http://www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint  T       EPSON   Stylus Photo 75
0       ESCPL2,BDC,D4   EPSON Stylus Photo 750  Annotation from <a href="mailto:olivier.delsol 
at usa dot net">\nOlivier Delsol</a>(03.16.1999):\n<br>\nThe 750 works with the upp files writt
en by DEPontius for the 740 and ghostscript 5.50. \n<br>See the Stylus 740 entry for a link.\n<
br>It could be perfect if the 6 inks were used !\n<br>6 color CMYKcm printer.
Epson   Stylus Photo EX Color   D       Ink Jet uniprint        ESC/P   1440    720     http://
www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint T       EPSON   Stylus Photo EX ESCPL2,
BDC             Printer only works with 4 colors. \nA3 paperformat works.\n\nWorks relatively w
ell with the Stylus Color 600 upp files.
Epson   Stylus Pro XL   Color   A       Ink Jet stcolor ESC/P   720     720     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Not as good as native N
T4 driver\non images, but quite useable; \ndon't forget to enable weaving; \nUniprint works als
o.
Fujitsu 1200    Color   A       Dot Matrix      epsonc  Proprietary     240     72      http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
Fujitsu 2400    Color   A       Dot Matrix      epsonc  Proprietary     240     72      http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
Fujitsu 3400    Color   A       Dot Matrix      epsonc  Proprietary     240     72      http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
Fujitsu PrintPartner 10V        BW      A       Laser   laserjet        PCL     2400    600
http://www.cs.wisc.edu/~ghost/printer.html      T                                       No driv
ers for the printer itself so just use the HP Laserjet. Works fine. No fuss.\n<br>Raju Kuallumk
al reports that the manual feed works fine, too.\n<br>Postscript option available.\n<br>2400 ve
rtical dpi smoothing of some sort?
Fujitsu PrintPartner 16DV       BW      A       Laser   ljet4   PCL     2400    600     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Postscript, Dup
lex, IrDA, Ethernet options available.
Fujitsu PrintPartner 20W        BW      A       Laser   ljet4   PCL     1200    1200    http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Ethernet, IrDA,
 Duplex, Postscript options.
Fujitsu PrintPartner 8000       BW      A       LED     ljet3   PCL     300     300     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Acts as a 8 Pag
e Per Minute HP LaserJet III
HP      2000C   Color   A       Ink Jet hpdj    PCL     600     600     ftp://ftp.sbs.de/pub/gr
aphics/ghostscript/pcl3/pcl3.html       T                                       300x300 and 600
x600 works, all print\nquality settings work, compressions\nwork.<br>This is a very fast printe
r;\nprobably 3x as fast as typical\ninkjets.<br>Output is very good \nquality - 30 to 45 sec fo
r a page\nwith a 6x6in photo and text.\n<br>\nHave not tried photos with linux.\n<br>Variants i
nclude Cse, Cxi, CN (network).
HP      2500C   Color   A       Ink Jet cdj550  PCL     600     600     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       CMYK printing.\n<br>Handles up 
to 13x19inch paper.\n<br>2500CM model includes 12M RAM, ethernet, Postscript 3.\n<br>2500Cse mo
del is US retail channel model.\n<br>2500Cxi model is US business channel model.\n<br>2500C+ in
cludes PCL 5 support.
HP      Color LaserJet 4500     Color   A       Laser   N/A     PostScript      600     600
N/A     T                                       Setup with RH 6.0 printtool\nusing the generic 
postscript\ndriver, prints in color.\nNice printer.
HP      DesignJet 650C  Color   B       Ink Jet dnj650c RTL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     F                                       Works "almost" - RGB da
ta are sent to the plotter.\nThe CMYK mode of the "B"-Models and the 75x Series\nis not support
ed. Images are somewhat\n"greenish"; gray is composed of CMY.\n \nNice for really LARGE printou
ts ...\n(ISO A0)
HP      Designjet 750 C Plus    Color   B       Ink Jet dnj650c RTL     600     600     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Works "almost" 
- RGB data are sent to the plotter. \n<br>The CMYK mode of the "B"-Models and the 75x Series is
 not supported. Images are somewhat "greenish"; gray is composed of CMY. Nice for really LARGE 
printouts ... (ISO A4-A0), (ANSI A, B...), (DIN C4, C3...)...\n<br>Driver: With HP RTL and Post
Script color resolution is always 300 x300. \n<br>Addressable 600 dpi using Sharp lines for vec
tor graphics in HP-GL/2.
HP      DeskJet 1000C   Color   D       Ink Jet pbm2ppa Proprietary     600     600     http://
www.httptech.com/ppa/   F                                       works in bw only.\n<br>11x17 fo
rmat printer.\n<br>Variants: Cse, Cxi.
HP      DeskJet 1100C   Color   B       Ink Jet cdj850  PCL     300     300     http://www.erdw
.ethz.ch/~bonk/hp850/hp850.html T                                       
HP      DeskJet 1120C   Color   B       Ink Jet hpdj    PCL     600     600     ftp://ftp.sbs.d
e/pub/graphics/ghostscript/pcl3/pcl3.html       T                                       Not tes
ted in any great detail, but does work.\n<br>Probably works with various color deskjet drivers 
including the cdj ones and hpdj.\n<br>Models Cse and Cxi are probably the same, with different 
bundled software.
HP      DeskJet 1200C   Color   A       Ink Jet pjxl300 PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       aka CopyJet\n<br>Postsc
ript option (but no longer sold by HP).\n<br>Also speaks HP-GL/2.\n<br>Probably also works with
 color ljet driver.
HP      DeskJet 1200C/PS        Color   A       Ink Jet pjxl300 PostScript      300     300
http://www.cs.wisc.edu/~ghost/printer.html      T                                       underst
ands PCL5, PJL.\n(probably identical to DeskJet 1200C\nplus PS cartridge)\nnot recommended for 
high-quality \nphoto printing
HP      DeskJet 1600C   Color   A       Ink Jet cdj1600 PCL     600     600     http://www.erdw
.ethz.ch/~bonk/hp850/hp850.html T                                       Only 300dpi in color.
HP      DeskJet 1600Cm  Color   A       Ink Jet N/A     PostScript      600     600     N/A
T                                       Also speaks PCL 5c; gs driver cdj1600.\n<br>Only 300dpi
 in color.
HP      DeskJet 310     Color   A       Ink Jet djet500 PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
HP      DeskJet 400     Color   A       Ink Jet cdj500  PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T       Hewlett-Packard HP DeskJet 400  PCL,C32         Must ch
ange cartridges to go from color to black and white.
HP      DeskJet 420C    Color   A       Ink Jet cdj550  PCL     600     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Color resolution is 300
x300dpi (color was not tested).\n<br>Manual change between color and B/W cartdriges.
HP      DeskJet 500     BW      A       Ink Jet djet500 PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Works just fine using g
hostscript.
HP      DeskJet 500C    Color   A       Ink Jet cdj500  PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Uses CMY or K catridge,
 user has to switch.
HP      DeskJet 510     BW      A       Ink Jet hpdj    PCL                     ftp://ftp.sbs.d
e/pub/graphics/ghostscript/pcl3/pcl3.html       T                                       Should 
work fine with various deskjet drivers.
HP      DeskJet 520     BW      A       Ink Jet hpdj    PCL     300     300     ftp://ftp.sbs.d
e/pub/graphics/ghostscript/pcl3/pcl3.html       T                                       works p
erfect b/w in rh 5.x 6.x with \nstandard setup via printtool
HP      DeskJet 540     Color   A       Ink Jet cdj500  PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       One printer cartridge a
t a time, black or CMY. Mostly like Deskjet 500.
HP      DeskJet 550C    Color   A       Ink Jet djet500 PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Works fine with ghostsc
ript.<br>\nI think this\nmodel uses both color and bw\ncartridges.  CMY+K?  CMYK?\n<br>Also wor
ks with uniprint driver cdj550.upp
HP      DeskJet 560C    Color   A       Ink Jet cdj550  PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
HP      DeskJet 600     Color   A       Ink Jet cdj550  PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Monochrome 600x600, col
or 300x300\n<br>\nCan use cdj500, cdj550 driver as\nwell for color support<br>\nMust specify to
 get 600x600\n resolution
HP      DeskJet 610C    Color   A       Ink Jet cdj670  PCL     600     600     http://www.erdw
.ethz.ch/~bonk/hp850/hp850.html T       HEWLETT-PACKARD DESKJET 610C    MLC,PCL,PML     Hewlett
-Packard DeskJet 610C   The Quality is quite OK, but not\nas good than with the original\nHP Dr
ivers under Windows.
HP      DeskJet 610CL   Color   A       Ink Jet cdj550  PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
HP      DeskJet 612C    Color   A       Ink Jet cdj550  PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T       HEWLETT-PACKARD DESKJET 610C    MLC,PCL,PML     Hewlett
-Packard DeskJet 610C   Far as I can tell everything's working fine\n<P>\n<em><a href="mailto: 
toby.cabot@pobox.com">toby.cabot@pobox.com</a>:</em> Text quality is\n<em>excellent</em> at 600
x600.  Seems like a good deal, especially as there's a $30 rebate \nso the net cost is around U
S$100.  \nThe comments in the\nghostscript docs about gamma correction apply here.  Photo image
s seem kinda muddy,\nespecially at 600x600.  If you figure out a way to fix this\ncleanly using
 RH driver scripts please let me know.\n<P>\nThe docs mention a special photo cartridge but I h
aven't tried that yet.\n<P>\nDon't forget - this printer (and pretty much any other this cheap)
 can't do better\n than 600x300 in color.
HP      DeskJet 660C    Color   A       Ink Jet hpdj    PCL     600     600     ftp://ftp.sbs.d
e/pub/graphics/ghostscript/pcl3/pcl3.html       F       HEWLETT-PACKARD DESKJET 660C    MLC,PCL
,PML    Hewlett-Packard DeskJet 660C    Known variants: Cse.
HP      DeskJet 670C    Color   A       Ink Jet cdj670  PCL     600     600     http://www.erdw
.ethz.ch/~bonk/hp850/hp850.html T                                       <br>A printhead alignme
nt tool called "powercolor" is available in metalab's system/printing directory.\n<br>Same prin
ter as 672C.
HP      DeskJet 672C    Color   A       Ink Jet cdj670  PCL     600     600     http://www.erdw
.ethz.ch/~bonk/hp850/hp850.html T                                       Works just fine, text i
s flawless\nWish I had pixel-for-pixel printing in Gimp,\nbut ps will have to do for now.\n<br>
A printhead alignment tool is available in metalab's system/printing directory.\n<br>Same print
er as 670C.
HP      DeskJet 682C    Color   A       Ink Jet cdj670  PCL                     http://www.erdw
.ethz.ch/~bonk/hp850/hp850.html T                                       Not exhaustively tested
, but it has done\neverything I've asked of it so far.
HP      DeskJet 690C    Color   A       Ink Jet cdj670  PCL     600     600     http://www.erdw
.ethz.ch/~bonk/hp850/hp850.html T       HEWLETT-PACKARD DESKJET 690C    MLC,PCL,PML     Hewlett
-Packard DeskJet 690C   A printhead alignment tool called "powercolor" is available in metalab'
s system/printing directory.
HP      DeskJet 692C    Color   A       Ink Jet cdj670  PPA     300     300     http://www.erdw
.ethz.ch/~bonk/hp850/hp850.html T       Unknown vendor  Unknown device                  only pr
oblem I've had was the separater page it prints every time I print some thing. After the SEOF w
as turned off ... problem solved.\nUsing COL2.3 with 2.2.13 kernel\n\nbase:   0x378\nirq:    no
ne\ndma:    none\nmodes:  SPP,EPP,ECP,ECPEPP,ECPPS2
HP      DeskJet 694C    Color   A       Ink Jet uniprint        PCL     600     600     http://
www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint T       HEWLETT-PACKARD DESKJET 690C
MLC,PCL,PML     Hewlett-Packard DeskJet 690C    Reported on Usenet to work fine; probably works
 with cdj850, too.\n<br>Printer does 600x300 in color.\n<br>690-694 should all work fine...\n<b
r>Fast text-only printing can be had with the hpdj driver.\n<br>High quality color photo printi
ng with the uniprint drivers.
HP      DeskJet 697C    Color   A       Ink Jet cdj550  PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Using LPRng with apsfil
ter as the if. \nSend all non-ps documents through a2ps, \nand then everything out through \ngh
ostview.Still need to setup color \nprintcap entries (I haven't had a \nneed yet) but I don't f
oresee any \nproblems with this.\n<br>Hardware only does 600x300 in color.
HP      DeskJet 710C    Color   D       Ink Jet pbm2ppa PPA     600     600     http://www.http
tech.com/ppa/   T       HEWLETT-PACKARD DESKJET 710C    SCP,VLINK       Hewlett-Packard Deskjet
 710 Series     Works in bw only.
HP      DeskJet 712C    BW      D       Ink Jet pbm2ppa PPA     600     600     http://www.http
tech.com/ppa/   F       HEWLETT-PACKARD DESKJET 710C    SCP,VLINK       Hewlett-Packard DeskJet
 710 Series     Works in bw only.\nCompile the ppm2ppa for a DeskJet 720C.\nUnder RedHat 6.0, l
ooks like 710C.\nWaiting for pnm2ppa for color support.
HP      DeskJet 720C    Color   D       Ink Jet pbm2ppa PPA     600     600     http://www.http
tech.com/ppa/   T       HEWLETT-PACKARD DESKJET 720     SCP,VLINK       Hewlett-Packard DeskJet
 720 Series     works in bw only.\nbw print is good quality!
HP      DeskJet 722C    BW      D       Ink Jet pbm2ppa Proprietary     600     300     http://
www.httptech.com/ppa/   T                                       works in bw only.
HP      DeskJet 810C    Color   A       Ink Jet cdj880  PCL     600     300     http://www.proa
xis.com/~mgelhaus/linux/software/hp880c/hp880c.html     T                               

HP      DeskJet 812C    Color   A       Ink Jet cdj880  PCL     600     600     http://www.proa
xis.com/~mgelhaus/linux/software/hp880c/hp880c.html     T                               
Colour resolution appears to be 300x300; Windows driver claims to software enhance this using "
PhotoRet II" - on Unix, unleash that GIMP. This printer appears to be functionally identical to
 the DeskJet 810C. I am currently using the 300dpi DeskJet 550C colour GhostScript setup as shi
pped with Red Hat Linux 6.0, and the output is excellent in every respect. I have marked the su
ggested driver as cdj880 as I am 99.9% certain it will work and this driver should give the bes
t output.
HP      DeskJet 820C    Color   D       Ink Jet pbm2ppa PPA     600     600     http://www.http
tech.com/ppa/   T                                       Works in bw only.\n<br>Variants: Cse, C
xi.
HP      DeskJet 832C    Color   B       Ink Jet cdj880  PCL     600     600     http://www.proa
xis.com/~mgelhaus/linux/software/hp880c/hp880c.html     T                               
Colors are a little off with the dj550 driver, but prints OK otherwise. Probably does not use t
he full resolution. Maybe other HP drivers will work better?\n<br>\nAccording to an <a href="ht
tp://www.deja.com/getdoc.xp?AN=561259557">article on deja.com</a>,\nthe DeskJet 832C is technol
ogically the same as the DeskJet 882C, only slower and with a reduced\nduty cycle.  Whether tha
t's true or not, it seems to print fine with the cdj880 driver in 300 or 600\ndpi.  However, th
e cdj880 driver's author's remarks (regarding quality on the DeskJet 882C printer)\ndo apply to
 the DeskJet 832C printer as well.\n<br>\nAnyone qualified to say if this printer qualifies for
 "Perfect" functionality with the cdj880 driver?
HP      DeskJet 850C    Color   A       Ink Jet cdj850  PCL     600     600     http://www.erdw
.ethz.ch/~bonk/hp850/hp850.html T       HEWLETT-PACKARD DESKJET 850C    PCL,MLC,PML     Hewlett
-Packard Deskjet 850C   
HP      DeskJet 855C    Color   A       Ink Jet cdj850  PCL     600     600     http://www.erdw
.ethz.ch/~bonk/hp850/hp850.html T       HEWLETT-PACKARD DESKJET 850C    PCL,MLC,PML     Hewlett
-Packard Deskjet 850C   
HP      DeskJet 870C    Color   B       Ink Jet cdj850  PCL     600     600     http://www.erdw
.ethz.ch/~bonk/hp850/hp850.html T       HEWLETT-PACKARD DESKJET 870C    PCL,MLC,PML     Hewlett
-Packard Deskjet 870C   Probably also works with hpdj generic pcl3 driver.\n<br>Colors are too 
light with known drivers.\n<br>This is the same printer as the 870Cxi and Cse.
HP      DeskJet 880C    Color   B       Ink Jet cdj880  PCL     600     600     http://www.proa
xis.com/~mgelhaus/linux/software/hp880c/hp880c.html     T                               
There is now a driver specifically for this printer; after a wee bit more tuning this printer w
ill qualify for "perfect".\n<br>With the cdj880 driver, it works well, but the colors are sligh
tly light overall. It needs to be corrected with\n-dGammaValC, -dGammaValM, -dGammaValY -dGamma
ValK and result can be nicely improved.
HP      DeskJet 882C    Color   A       Ink Jet cdj880  PCL     600     600     http://www.proa
xis.com/~mgelhaus/linux/software/hp880c/hp880c.html     T                               
Should work perfectly with the cdj880 driver; let me know!\n<br>Works well with hpdj at 300dpi.
\n<br>Works OK at 600dpi with cdj850 driver, although colors are faded.\n<br>Brian, a Usenet co
ntributor, is working with the hpdj author to improve support.\n<br>This is the <b>same printer
</b> as the 880, with a retail box and idiot consumer software.
HP      DeskJet 890C    Color   A       Ink Jet cdj890  PCL     600     600     http://www.erdw
.ethz.ch/~bonk/hp850/hp850.html T       HEWLETT-PACKARD DESKJET 890C    PCL,MLC,PML     Hewlett
-Packard Deskjet 890C   Works well with this driver, and probably reasonably well with other cd
j and hpdj drviers.\n<br>Minor color variations from Windows driver.
HP      DeskJet 895C    Color   B       Ink Jet cdj880  PCL     600     600     http://www.proa
xis.com/~mgelhaus/linux/software/hp880c/hp880c.html     T                               
Should work perfectly with Matthew Gelhaus's driver; let me know.\n<br>People report color oddi
ties of various types with hpdj and cdj850.
HP      DeskJet 895Cxi  Color   B       Ink Jet cdj880  PCL     600     600     http://www.proa
xis.com/~mgelhaus/linux/software/hp880c/hp880c.html     T                               
Also usually works with hpdj.
HP      DeskJet 970C    Color   B       Ink Jet cdj550  PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       in redhat 6.0 works the
 driver perfect \n<br>reported trouble in SuSE; pilot error?\n<br>Should work at least mostly.\
n<br>Same printer as 970Cxi, etc...\n<ef>With GS5.50 and CDJ550 SUSE 6.3 printer\n    works fin
e color too dark but color
HP      DeskJet 970Cse  Color   B       Ink Jet cdj550  PCL     2400    1200    http://www.cs.w
isc.edu/~ghost/printer.html     T       HEWLETT-PACKARD DESKJET 970     MLC,PCL,PML     Hewlett
-Packard DeskJet 970C   Sorry for the verbosity.  The 2400x1200 mode doesn't work in Linux or\n
Solaris, just windoze; under unices, the best resolution is 600x600\n(I *think* I'm getting 600
x600, only because when setting the print\nmode quality under windoze and then printing from li
nux via samba, the\nimage output at 300x300 and 600x600 appear slightly different under a\nstro
ng magnifying glass.  On my RH 6.0 machine, I am using the driver\n"HP Deskjet 550C/560C/6xxC s
eries" and it works perfectly.  I am printing\nto this printer which is on a Win98 (yawn) box t
hrough Samba and have had\nno problems whatsoever. On my 6.1 box (from the office over an ISDN 
line)\nI am also printing to this printer via samba, but am using the 870 driver \nthere and it
 works just as well as if not better than the one I'm using \nfrom my 6.0 box. This printer doe
s automatic 2-sided printing, and if I \ngo into the Win98 box and turn this feature on and the
n send my print job,\nabout half the time it works just fine (I have *no* idea why it works 50%
\nof the time -- it either ought to work all the time or never).  The colors\nappear a little d
ifferent from the windoze colors; HOWEVER, I have tuned\nmy colors on the NEC 6FGp monitor to m
atch them and they haven't change\na bit in 3 weeks.  This printer in b&w mode is significantly
 faster\nthan my trusty HP LJ4M laser printer (but postscript is not available\neven as an opti
on, sigh.), so I believe the claim of up to 12ppm in b&w.\nOn my next system, I'm installing th
is on my LINUX box via the USB port\n-- I'll post when that either works or doesn't...  Somewha
t pricey (~\n$400), but built like a sherman tank.  Refill: 2 cartridges -- 1 black,\n1 3color 
More info.  Driver: Ghostscript: same as for 970Cxi (see above) #\n(600x600 Color) Autoprobe in
fo -- no idea.
HP      DeskJet670c     Color   A       Ink Jet deskjet PPA     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     F                                       
HP      LaserJet        BW      A       Laser   laserjet        PCL     300     300     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
HP      LaserJet 1100   BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Overall, a nice, pretty
, cheap (under $400 US) laser.\n<br>This printer is slow with Ghostscript versions prior to 5.5
0.
HP      LaserJet 1100A  BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Same as 1100 with a cop
ier/scanner.\n\n"ljet4" yields much better prints\nthan "lj4dith".\n\nYou need to enable "fix s
tair-\nstepping text".
HP      LaserJet 2      BW      B       Laser   laserjet        PCL     300     300     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       ASCII comes out
 fine.<BR>\n - send eof after job (yes)<BR>\n - Fix stair stepping (yes)<BR>\n<P>\nUsing ghosts
cript and HP Laserjet driver, pages are shifted up a little (1/8 inch).<BR>\n - experimentation
 with margins does not seem to help<BR>\n<P>\nSending windows jobs via samba works excellent!
HP      LaserJet 2 w/PS BW      A       Laser   N/A     PostScript      300     300     N/A
T                                       Slow, but perfectly functional with 2.5 megs of memory.
\n<br>Not really a IIM, but a II with ps cartridge.
HP      LaserJet 2100   BW      B       Laser   ljet4   PCL     1200    1200    http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Ghostscript drives at 6
00dpi.
HP      LaserJet 2100M  BW      A       Laser   N/A     PostScript      1200    1200    N/A
T       Hewlett-Packard HP LaserJet 2100 Series PJL,MLC,PCL,PCLXL,POSTSCRIPT    Hewlett-Packard
 LaserJet 2100 Series   postscript level 2 printer, 1200dpi...\nthat's all what is needed.
HP      LaserJet 2D     BW      A       Laser   ljet2p  PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
HP      LaserJet 2P     BW      A       Laser   ljet2p  PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Definitely requires mor
e then the default amount of memory (256k?) to do 300 dpi postscript, full page.\nCertainly wor
ks at 150dpi.\nASCII:Send EOF & fix stair-stepping.
HP      LaserJet 3      BW      A       Laser   ljet3   PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Make sure you have at l
east 2MB\nRAM, otherwise it won't be able to\nprint complex pages.\n<br>\nWidely available used
 this workhorse\njust keeps going and going. It also uses about the cheapest toner available. R
emanufactured toner is about $25,\nand Xerox sells new units for about $50. Cheaper to run than
 ANY inkjet. Keep your eyes open for used HP LJIII\nor IIID's they are a good buy.
HP      LaserJet 3100   BW      F       Laser   N/A     Proprietary     600     600     N/A
F       Hewlett-Packard HP LaserJet 3100        HP GDI  Hewlett-Packard LaserJet 3100 MFP
Multifunction device - copier/scanner/fax/etc.\n<br>PCL 4 emulation appears to be host-based.\n
<br>Known variants: xi (US retail), se (US business).  3100<nosuffix> is international model.\n
Not usable with Linux: an HP tech support person told\nme that half of the 3100's functionality
 is handled\nby the Windows-only drivers.  Thus, even if you were\non a Windows machine and go 
to the DOS prompt, doing\n`DIR > LPT1:' will fail; everything has to go through\nthe windows dr
ivers. (sigh)
HP      LaserJet 3D     BW      A       Laser   ljet3d  PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
HP      LaserJet 3P w/PS        BW      A       Laser   N/A     PostScript      300     300
N/A     F                                       Same as a IIIP, just with a postscript cartridg
e.\n<br>Under Red Hat "install a printer" and select "PostScript" as the printer.  \n<br>lpr in
stalls and uses the Gnu Magicfilter by default.  \n<br>High quality print.  \n<br>Some pdf docu
ments do not translate to a PostScript file that this printer can print, even if Level 1 is exp
licitly selected on the xpdf command line.  Same result with Adobe Acroreader.  May be related 
to the age of the PS cartridge (1991).
HP      LaserJet 4 Plus BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
HP      LaserJet 4050N  BW      A       Laser   N/A     PostScript      1200    1200    N/A
T                                       Great printer for heavy printing needs.\nFast, reliable
, jamm proof.\nHandles duplex/simplex great with linux.\n<BR><BR>\nBasically a great workhorse 
of a printer.
HP      LaserJet 4L     BW      A       Laser   ljet4   PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T       Hewlett-Packard LaserJet 4L     HP ENHANCED PCL5,PJL
        Has worked with Linux for over 3 years.\n<br>Probably not in production anymore.
HP      LaserJet 4M     BW      A       Laser   N/A     PostScript      600     600     N/A
T                                       Network printer\nNot tested much, but seems to work
HP      LaserJet 4ML    BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
HP      LaserJet 4P     BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Works pretty good.  KDE
 has an HP Laserjet utility app you can try.
HP      LaserJet 5      BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
HP      LaserJet 5000   BW      A       Laser   N/A     PostScript      1200    1200    N/A
T                                       PostScript L2,\nPCL5, PCL6\n<br>4/8/12MB std, depending
 on flavour.\nWide Format printer - if the paper size exists, it can print on it up to Ledger a
nd A3\n<br> \nOptional accessories are duplexer, 250 sheet tray, 500 sheet tray, hard drive, ex
tra memory\n<br>\nWorks well with the generic Postscript Driver
HP      LaserJet 5L     BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T       Hewlett-Packard HP LaserJet 5L  HP ENHANCED PCL5,PJL
Hewlett-Packard LaserJet 5L Printer     Remember that the 5L is more\nlike the Ljet4 family tha
n the\nLjet5 family (as with most ?L\nprinters from HP). \n<br>Getting more RAM is highly recom
mended; with only the standard 1Meg you may be unable to print some jobs.\n<br>From command lin
e the 1st line of the first page is only half visible; also col 77 is the last column to be ful
ly printed. Sophisticated apps that use wide margins all-around print fine.
HP      LaserJet 5M     BW      A       Laser   N/A     PostScript      600     600     N/A
T                                       Works fine.
HP      LaserJet 5MP    BW      A       Laser   N/A     PostScript      600     600     N/A
T                                       
HP      LaserJet 5P     BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T       Hewlett-Packard HP LaserJet 5P  HP ENHANCED PCL5,PJL
Hewlett-Packard LaserJet 5P Printer     
HP      LaserJet 6      BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
HP      LaserJet 6L     BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T       Hewlett-Packard HP LaserJet 6L          Hewlett-Packard
 LaserJet 6L Printer    
HP      LaserJet 6MP    BW      A       Laser   N/A     PostScript      600     600     N/A
T       Hewlett-Packard HP LaserJet 6MP PJL,MLC,PCLXL,PCL,POSTSCRIPT    Hewlett-Packard LaserJe
t 6MP Printer   Only 800K of the first 1M SIMM is \navailable for PS, the rest is a ROM\ninterp
reter.  A 2nd 16M SIMM is \nrecommended.  Standard EDO or fast\npage SIMMs do not work.  This \
nprinter ejects a blank page if ^D \nis received.  This is unlike the \n4MP.  Magicfilter-1.2 \
nsetup: copy ljet4m-filter and\nreplace the word "postscript" by \n"cat".
HP      LaserJet 6P     BW      B       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T       Hewlett-Packard HP LaserJet 6P  HP ENHANCED PCL5,PJL
Hewlett-Packard LaserJet 6P Printer     
HP      LaserJet 8000   BW      A       Laser   N/A     PostScript      1200    1200    N/A
T                                       PS L2, PCL5, PCL6\n24 ppm 1200dpi evolution of 5Si prin
ter\n\nOptional accessories include 2X500sheet feeder, 2000sheet feeder, 8 bin mailbox, 5bin ma
ilbox w/stapler, hard drive, additional memory, duplexer, and envelope feeder\n\nIt's a big, he
avy printer, don't even think about putting it on a card table.  \nExceptional b/w output.  Cur
rently using with the Generic Postscript driver
HP      LaserJet 8100   BW      A       Laser   N/A     PostScript      1200    1200    N/A
T                                       PS L2, PCL5, PCL6, 1200dpi 32ppm \n\nsame optional acce
ssories as 8000, Internals are of a newer design.\n\nSame size as LJ8000, too.  Other than soun
ding like a 737 winding up its turbines, this is one rock solid printer.  This printer also pro
cesses jobs quickly, throw your most complex jobs at this one.
HP      LaserJet Plus   BW      A       Laser   ljetplus        PCL     300     300     http://
www.cs.wisc.edu/~ghost/printer.html     F                                       
HP      Mopier 320      BW      A       Laser   N/A     PostScript      1200    1200    N/A
T                                       Same as the 8100, except with ALL of the accessories AN
D an additional unit on top that allows copying to be done directly at the printer.  Like the 8
000 and 8100, this mopier handles up to A3 and Ledger size media.  Now if you could only fax fr
om it...
HP      OfficeJet 500   Color   D       Ink Jet cdj550  PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Multifunction device (c
opier/scanner/fax/etc).\n<br>Scanner, etc probably don't work, printing ought to.
HP      OfficeJet 600   Color   D       Ink Jet cdj550  PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       A multifunc machine: sc
an/printer/copy/fax\nI've only got printing to works under Linux.\nColor output & B/W using GS 
driver without\nany special setup - with good ouput.\n<Br>\nSuggest: read on Gamma correction o
n:\n/usr/doc/gs/devices.txt.gz
HP      OfficeJet 625   Color   D       Ink Jet hpdj    PCL     600     600     ftp://ftp.sbs.d
e/pub/graphics/ghostscript/pcl3/pcl3.html       T                                       Multifu
nction device: printer/scanner/copier/fax.\n<br>Head alignment/cleaning impossible with usual f
ree software.\n<br>Scanning not supported.\n<br>Faxing and copying can be done from the front p
anel.
HP      OfficeJet Pro 1150C     Color   D       Ink Jet hpdj    PCL     600     600     ftp://f
tp.sbs.de/pub/graphics/ghostscript/pcl3/pcl3.html       T                               
Multifunction Device - Color Priter/Scaner/Copier
HP      OfficeJet Pro 1170Cse   Color   B       Ink Jet N/A     PostScript      300     300
N/A     F                                       Configuration with X printertool :\nUsing smb(s
amba) to share printer.\nSet hostname of printer server\nset ip address of printer server\nset 
printer name from server\nset workgroup name\nselect DeskJet 550C = this one\nhas several quali
ty option\nselect to place a caridge return\nafter each line
HP      PaintJet        Color   A       Ink Jet pj      PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
HP      PaintJet XL     Color   A       Ink Jet pjxl    PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
HP      PaintJet XL300  Color   A       Ink Jet pjxl300 PCL                     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
HP      PhotoSmart P1100        Color   D       Ink Jet cdj550  PCL     2400    1200    http://
www.cs.wisc.edu/~ghost/printer.html     T       HEWLETT-PACKARD PHOTOSMART P1100        MLC,PCL
,PML    Hewlett-Packard PhotoSmart P1100        I think this is basically the same printer\nas 
the DeskJet 970Cse, except it has\nports on the side for inserting SmartMedia\nand CompactFlash
 digital "film" and has an\nLCD panel that lets you select pictures from\nthe "film" to print.\
n\nDuplex is missing; the resolution is limited to 600x600.  I'm planning on playing with writi
ng a CUPS driver for this printer.
IBM     3853 JetPrinter BW      A       Dot Matrix      jetp3852        Proprietary     82
82      http://www.cs.wisc.edu/~ghost/printer.html      T                               

IBM     4019    BW      A       Laser   laserjet        PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Works great on RedHat 5
.2.  Used \nprinttool to set it as an HP LaserJet.\n500k RAM is too little for a PostScript\nim
age (using Ghostscript).  Using \na 4Mbyte memory board ($95 board not\nplug in card) Postscrip
t prints great, \nbut slow.  A Postscript option is \navailable for this printer, but the\nopti
on was not used in this evaluation.
IBM     4029 10P        BW      A       Laser   N/A     PostScript      600     600     N/A
T                                       My only setup is as a postscript \nprinter.  However th
e printer also\nhas HP Laser PCL support. My setup \nis just out-of-the-box Redhat 5.2\npostscr
ipt support.
IBM     4303 Network Color Printer      Color   A       Laser   N/A     PostScript      600
600     N/A     T                                       Used the RH6.0 printtool to configure t
his.\nPrinter was configured as a lpd network resource\nusing the postscript option at 600x600.
 \nThe RH postscript test page printed perfect.
IBM     Page Printer 3112       BW      A       Laser   N/A     PostScript      1200    1200
N/A     T                                       Built by Lexmark, similar to \nOptra R/R+. PPD 
driver lexoptra.ppd \navailable at Lexmark's web site in \nthe file PS_PPD.TAR, includes \nsupp
ort for about all printer \nspecialties. Printcap filters only\navailable for generic 600 dpi P
S \nprinter, as far as I know.\nSupports also HP PCL 5, emulates \nLaserjet 4.
IBM     ProPrinterII    BW      A       Dot Matrix      N/A     ASCII   120     120     N/A
T                                       Easy to get working on serial - just dump straight ASCI
I.\n\nIf you have the parallel module installed, you need to use\nmagicfilter which has a ProPr
inter / ProPrinterII specific\nfilter which fixes some minor oddities.\n\nFair warning; the ser
ial interface drops characters frequently.\nDon't use it on the network if you can avoid it. ;)
Kyocera F-3300  BW      A       Laser   ljetplus        PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       1.5M RAM\nWorks, no pro
blem.
Kyocera FS-1700+        BW      A       Laser   N/A     PostScript      600     600     N/A
T                                       PCL5 support \n<br>Ethernet option\n<br>2M RAM std; use
s regular SIMMS
Kyocera FS-3500 BW      B       Laser   ljet3   PCL     300     300     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       slightly garbled output from gh
ostscript when printing postscript files using Ljet3 driver\nworks fine on samba print server w
ith clients using Ljet3 or KY-3500 drivers\nno problems with WordPerfect 8 drivers - either Lje
t3 or KY-3500\n\nethernet option still a mystery - but can ftp text to print!\ndon't know how t
o change IP address\nsends aarp packets on boot\n\n3mb RAM - proprietry SIMMS?
Kyocera FS-3750 BW      A       Laser   ljet4   PCL     1200    1200    http://www.cs.wisc.edu/
~ghost/printer.html     T                                       optionally available: 10MBit or
 100MBit Network-Module with 32MB RAM, works fine with linux
Kyocera FS-600  BW      A       Laser   ljet4   PCL     600     600     http://www.cs.wisc.edu/
~ghost/printer.html     T       Kyocera FS-600  PCL5E,PJL       Kyocera FS-600  Postscript opti
on\n<br>4MB standard
Kyocera FS-800  BW      A       Laser   N/A     PostScript      600     600     N/A     T
                                Also works fine as LJ4 clone -\nPCL5?
Kyocera P-2000  BW      A       Laser   ljetplus        PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       1.5M RAM\nWorks, no pro
blem
Lexmark 1000    Color   B       Ink Jet lm1100  Proprietary     600     600     http://209.233.
17.85/lexmark/  F                                       Tim's driver runs this at 300dpi, color
.
Lexmark 1020    Color   F       Ink Jet N/A     Proprietary     600     300     N/A     F
                                This, the model 4078-002, is a paperweight.\n<br>The "1020 Busi
ness", model 4078-001, has onboard PCL, and works.
Lexmark 1020 Business   Color   D       Ink Jet hpdj    PCL     600     300     ftp://ftp.sbs.d
e/pub/graphics/ghostscript/pcl3/pcl3.html       T                                       This bu
siness edition, model 4078-001, has PCL.\n<br>Known to print fine at 300dpi, black-only.\n<br>W
orks in color, too.\n<br>Note that the non-business 1020 does <b>not</b>, and is a paperweight.
\n<br>This printer is much slower in PCL mode than in proprietary mode.
Lexmark 1100    Color   B       Ink Jet lm1100  Proprietary     600     600     http://209.233.
17.85/lexmark/  F                                       Tim Engler's driver supports 300dpi col
or; it's a ppm-to-lexmark filter.\n<br>\nHenryk's l7k driver may support this in the future.<br
>\nUnsuited to Linux-only computers, as\nyou can't change cartridges without its\nWindows contr
ol program. Compatibility\npoor.
Lexmark 2030    Color   D       Ink Jet pbm2l2030       Proprietary     1200    1200    http://
move.to/$HOME   F                                       Known to work at 300dpi BW with Tom Pae
tzold's pbm driver.
Lexmark 2050    Color   F       Ink Jet N/A     Proprietary     600     600     N/A     T
                                
Lexmark 2070    Color   B       Ink Jet c2070   Proprietary     600     600     http://www.stud
et.fh-muenster.de/~kornblum/Lexmark2070.html    F                                       The Lex
mark2070 package can drive this printer in 300dpi;\nthere are two different drivers for bw and 
or CMYK color.
Lexmark 3000    Color   B       Ink Jet cdj550  PCL     600     300     http://www.cs.wisc.edu/
~ghost/printer.html     T       Lexmark Lexmark 3000 Color Jetprinter   LEXWPS,PCL3     Lexmark
 3000 Color Jetprinter  600x300 DPI printing mode and\nphoto cartridge not supported.\nUse 300x
300 DPI.\n\nA previous entry said this printer\ndoesn't work at all:\n"will not print at all, s
ends all\nprint jobs to spool and I can not\nthen print from the spool" - maybe\nsome older gho
stscript versions\ndon't work, or maybe it was just\na misconfiguration.
Lexmark 3200    Color   F       Ink Jet pbm2l7k Proprietary     1200    1200    http://bimbo.fj
fi.cvut.cz/~paluch/l7kdriver/   F                                       Known not to work with 
Henryk's pbml7k driver.\n<br>However, someone has recently forwarded him documentation, so ther
e may be hope!
Lexmark 4039 10plus     BW      A       Laser   N/A     PostScript      600     600     N/A
T       Lexmark International   IBM LaserPrinter 4039 plus      PCL 5 Emulation, PostScript Lev
el 2 Emulation, NPAP, PJL               Also does PCL 5e.
Lexmark 5000    Color   B       Ink Jet lx5000  Proprietary     600     1200    http://www.powe
rup.com.au/~pbwest/lexmark/lexmark.html F                                       Works in color 
and bw at 600 & 300dpi with Peter West's driver.&nbsp;\nBidirectional/1200dpi printing is not y
et supported.\n<br>Alignment and head cleaning supported by sending data files to the\nraw devi
ce.  These files available at the driver site.\n<br>DryingTime an black-only run time parameter
s.\n<br>The photo cartridge in not supported.\n<br>A description of the protocol is at http://w
ww.powerup.com.au/~pbwest/lexmark/protocol.html\n<br>Also works about as well with the Lexmark 
7000 driver kit; see that printer for more information.
Lexmark 5700    Color   B       Ink Jet lxm5700m        Proprietary     1200    1200    http://
www.ultranet.com/~setaylor/papers.htm   F                                       Stephen Taylor 
wrote this driver, which can print at 1200dpi in bw only.\n<br>Henryk's l7k driver can run it a
t 600x600 or 600x300, including color.  Maybe 1200dpi, too?
Lexmark 7000    Color   B       Ink Jet pbm2l7k Proprietary     1200    1200    http://bimbo.fj
fi.cvut.cz/~paluch/l7kdriver/   F                                       600 dpi printing seems 
to work with \nHenryk's new driver
Lexmark 7200    Color   B       Ink Jet pbm2l7k Proprietary     1200    1200    http://bimbo.fj
fi.cvut.cz/~paluch/l7kdriver/   F                                       Known to work with Henr
yk's l7k program; 600x600 or 600x300 only.
Lexmark Optra Color 1200        Color   A       LED     N/A     PostScript      600     600
N/A     T                                       CMYK color printing.\n<br>Also speaks PCL 5c an
d 6.\n<br>Model 1200n includes network port.
Lexmark Optra Color 1275        Color   A       Laser   N/A     PostScript      600     600
N/A     T                                       Also speaks PCL 5c and 6.\n<br>Model 1275n incl
udes network port.\n<br>CMYK color printing.
Lexmark Optra Color 40  Color   A       Ink Jet N/A     PostScript      600     600     N/A
T       Lexmark International   Lexmark OptraColor 40   PCL 5 Emulation, PostScript Level 2 Emu
lation, NPAP, PJL       Lexmark OptraColor 40   Also groks PCL 5c.\n<br>Model 40n includes netw
ork interface and 12M RAM.\n<br>CMYK or CMYKcm color printing.\n<br>Print head loading/alignmen
t/etc requires <a href="/~gtaylor/download/printing/cartutil.c">special software</a>.\n<br>Uses
 generic EDO or FPM DRAM SIMM.
Lexmark Optra Color 45  Color   A       Ink Jet N/A     PostScript      600     600     N/A
T                                       Also speaks PCL 5c.\n<br>CMYK or CMYKcm color printing.
\n<br>Cartridge loading/alignment/etc requires <a href="/~gtaylor/download/printing/cartutil.c"
>special software</a>.\n<br>Model 45n includes network interface.
Lexmark Optra E BW      A       Laser   ljet4   PCL     600     600     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       
Lexmark Optra E+        BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
Lexmark Optra E310      BW      A       Laser   N/A     PostScript      600     600     N/A
T                                       A postscript printer, so works fine.\n<br>Std 2M is oka
y, but should add more\n<br>holds upto 66Mb, takes standard 72pin SIMM\n<br>Also speaks PCL5e
Lexmark Optra Ep        BW      A       Laser   N/A     PostScript      600     600     N/A
T       Lexmark International   Lexmark Optra Ep LaserPrinter   PCL 5 Emulation,PostScript Leve
l 2 Emulation,NPAP,PJL  Lexmark Optra Ep LaserPrinter   a true Postscript printer<br>\n2M ram e
xpandable to 6M
Lexmark Optra K 1220    BW      A       Laser   N/A     PostScript      600     600     N/A
T                                       A Postscript printer, so should(tm) work fine.\n<br>2M 
RAM std; holds up to 66M.\n<br>Also speaks PCL 6.
Lexmark Optra R+        BW      A       Laser   N/A     PostScript      1200    1200    N/A
T                                       Also speaks PCL level 5.\nFor 1200DPI use Postscript Le
vel 2.\nStd 2MB RAM adequate for 600DPI\nPostscript, RAM is expandable to\n64MB using 72pin SIM
Ms.
Lexmark Optra S 1250    BW      A       Laser   N/A     PostScript      1200    1200    N/A
T       Lexmark International   Lexmark Optra S 1250    PCL 6 Emulation, PostScript Level 2 Emu
lation, NPAP, PJL       Lexmark Optra S 1250    Also speaks PCL level 6.<br>\nModel 1250n inclu
des network port.<br>\nModel 1255 is the successor and has\nsome enhanced software features e.g
. N-up printing
Lexmark Optra S 1855    BW      A       Laser   N/A     PostScript                      N/A
T                                       Also does PCL 5 without problems; don't try PCL 6.\n<p>
 Note Lexmark sends a Winduhs driver, but you can set it up as plain Postscript without problem
s.
Lexmark Valuewriter 300 BW      A       Laser   ljet2p  PCL                     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Reported to work well a
s a laserjet II on Usenet.\n<!-- by rob at sput.webster.nl -->
Lexmark Winwriter 100   BW      F       Ink Jet N/A     Proprietary                     N/A
T                                       
Lexmark Winwriter 150c  Color   F       Ink Jet N/A     Proprietary     600     300     N/A
T                                       Well, I'm trying to figure out\nhow to get this working
, but\nI think that this printer is a \npaperweight.
Lexmark Winwriter 200   BW      F       Laser   N/A     Proprietary     300     300     N/A
F                                       
Lexmark Winwriter 400   BW      D       Laser   ljetplus        PCL     600     300     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Emulates HP Las
erjet II under any OS, but needs Windows driver to get 600x300 dpi.
Lexmark Z11     Color   F       Ink Jet N/A     Proprietary     1200    1200    N/A     T
Lexmark Lexmark Z11     LOWENDLEXCPD    (none)  No mention of PCL, etc; I'm guessing paperweigh
t.\n<br>Remote chance of it working with current Linux drivers for the 7000, 3200, or 5700.<br>
\nSee http://bimbo.fjfi.cvut.cz/~paluch/l7kdriver/
Lexmark Z51     Color   D       Ink Jet pbm2l7k Proprietary     1200    1200    http://bimbo.fj
fi.cvut.cz/~paluch/l7kdriver/   F       Lexmark kernel: Lexmark Z51     LNPAP   Lexmark Z51
Known to work with Henryk's l7k driver.
Minolta PagePro 6       BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T       MINOLTA PagePro 6       HP ENHANCED PCL5        

Minolta PagePro 6e      BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
Minolta PagePro 6ex     BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T       Minolta PagePro   6e    HP ENHANCED PCL5e,PJL,WinStyler
        Graphics turn out too dark.
Minolta PagePro 8       BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
Minolta PagePro 8L      BW      D       Laser   ljet2p  PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T       Minolta PagePro 8L      PrintGear;PCL;PLJ       
PCL 4.5 printing is limited to 300dpi.
NEC     P2X     BW      A       Dot Matrix      uniprint        ESC/P   360     360     http://
www.cs.wisc.edu/~ghost/aladdin/doc/Devices.htm#Uniprint T                               
uniprint parameter file nxp2x.upp
NEC     PinWriter P6    Color   A       Dot Matrix      necp6   Proprietary     360     360
http://www.cs.wisc.edu/~ghost/printer.html      T                                       Color i
s an optional feature for this printer.\nBasically you could by a replacement part to enable co
lor printing.
NEC     PinWriter P6 plus       Color   A       Dot Matrix      necp6   Proprietary     360
360     http://www.cs.wisc.edu/~ghost/printer.html      T                               
Color is an optional feature for this printer.\nBasically you could by a replacement part to en
able color printing.
NEC     PinWriter P60   Color   A       Dot Matrix      necp6   Proprietary     360     360
http://www.cs.wisc.edu/~ghost/printer.html      T                                       Color i
s an optional feature for this printer.\nBasically you could by a replacement part to enable co
lor printing.
NEC     PinWriter P7    Color   A       Dot Matrix      necp6   Proprietary     360     360
http://www.cs.wisc.edu/~ghost/printer.html      T                                       Color i
s an optional feature for this printer.\nBasically you could by a replacement part to enable co
lor printing.
NEC     PinWriter P7 plus       Color   A       Dot Matrix      necp6   Proprietary     360
360     http://www.cs.wisc.edu/~ghost/printer.html      T                               
Color is an optional feature for this printer.\nBasically you could by a replacement part to en
able color printing.
NEC     PinWriter P70   Color   A       Dot Matrix      necp6   Proprietary     360     360
http://www.cs.wisc.edu/~ghost/printer.html      T                                       Color i
s an optional feature for this printer.\nBasically you could by a replacement part to enable co
lor printing.
NEC     SilentWriter LC 890     BW      A       Laser   N/A     PostScript      300     300
N/A     F                                       Also supports LaserJet (PCL 2 I believe) but wh
y bother.\nNEC website has very detailed information (FAXBACK) in PDF format to be downloaded f
or free.\nLists all the parts and part numbers.
NEC     Silentwriter2 S60P      BW      A       Laser   N/A     PostScript      300     300
N/A     T                                       There is almost nothing that will\nstop this pr
inter.  I got mine several years ago\nfrom an office upgrade; it was already ancient at\nthat t
ime.  But, as a Postscript printer, it \nworks great with Linux.  But of course it is \nvery sl
ow.  It also emulates the HP LaserJet II.
NEC     Silentwriter2 model 290 BW      A       Laser   N/A     PostScript      300     300
N/A     T                                       Comes with two megs of memory, fully functional
 but slow.
NEC     SuperScript 100C        Color   D       Ink Jet hpdj    PCL     600     300     ftp://f
tp.sbs.de/pub/graphics/ghostscript/pcl3/pcl3.html       T                               
Printer only capable of 300x300 in color.\n<br>Should(tm) work perfectly; change this if so!\n<
br>CMY 3 color printing, or black-only w/black cart.
NEC     SuperScript 1260        BW      D       Laser   ljet2p  PCL     600     600     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Only prints at 
300dpi with PCL.\n<br>Model 1260N includes network interface.
NEC     SuperScript 150C        Color   D       Ink Jet hpdj    PCL     600     300     ftp://f
tp.sbs.de/pub/graphics/ghostscript/pcl3/pcl3.html       T                               
Should(tm) work perfectly; if so change this entry.\n<br>CMY color printing or black-only.
NEC     SuperScript 610plus     BW      F       LED     N/A     Proprietary     300     300
N/A     F                                       Traditional "GDI" printer; PCL and PS level 1 s
upport for apps running in Windows DOS box.
NEC     SuperScript 650C        Color   D       Ink Jet hpdj    PCL     1200    600     ftp://f
tp.sbs.de/pub/graphics/ghostscript/pcl3/pcl3.html       T                               
Should(tm) work perfectly; change this entry if so!\n<br>CMYK four-color printing; separate bla
ck for text.
NEC     SuperScript 660 BW      F       Laser   N/A     Proprietary     600     600     N/A
F                                       Clearly described as a "GDI" printer on NEC's page.
NEC     SuperScript 660i        BW      A       Laser   ljet4   PCL     600     600     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       I set up the NE
C SuperScript 660i using the RedHat print tool with HP Laserjet 4/5/6 selected.  Works perfectl
y.
NEC     SuperScript 660plus     BW      F       Laser   N/A     Proprietary     600     600
N/A     T                                       PCL 5e emulation appears to be implemented in t
he Windows driver.
NEC     SuperScript 750C        Color   D       Ink Jet hpdj    PCL     600     600     ftp://f
tp.sbs.de/pub/graphics/ghostscript/pcl3/pcl3.html       T                               
CMYK four-color printing or black-only; two heads.\n<br>Should(tm) work perfectly; if you have 
one and it does, change this entry!
NEC     SuperScript 860 BW      D       Laser   ljet2p  PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Only prints at 300dpi w
ith PCL.\n<br>
NEC     SuperScript 870 BW      D       Laser   ljet2p  PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Prints at 300x300 with 
PCL.\n<br>There is a PostScript option, alas it is a Windows driver Postscript emulation, so wo
n't help.\n<br>Also speaks "PrintGear".\n<br>Network option.
Oce     3165    BW      A       Laser   N/A     PostScript      600     600     N/A     T
                                Copier, but also a network printer. Also supports PCL. Username
s are correctly identified for the mailbox.\nVery good for 10k to 100k prints/month.\nGeneric P
ostscript doesn't support the automatic stapler.
Okidata 8p      BW      A       LED     ljet4   PCL     600     600     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       This printer is mostly HP 6L co
mpatible.\nSo, no big problems. Setting the permanent\nsettings with HP PJL is still unsupporte
d\nunder Linux. This is a general problem,\nnot only related to Oki 8p.
Okidata ML 192+ BW      D       Dot Matrix      ibmpro  ASCII   120     120     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Print quality looks bes
t when setup for 60x60\nresolution. RedHat users need to setup this printer\nin the file /var/s
pool/lpd/lp/postscript.cfg\nOtherwise you're stuck with plain text only.\nThis printer like oth
er older Okidata printers\ncomes in two flavors, IBM and Okidata emulation.\nI've only tried to
 use the IBM flavor.\nThose that have the Okidata emulation need to \nread the instructions to 
recompile Ghostscript\nto include the oki182 driver as it is not included \nby default.
Okidata Microline 182   BW      B       Laser   oki182  Proprietary     144     144     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
Okidata OL 400w BW      B       Laser   oki4w   Proprietary                     http://www.pica
nte.com/~gtaylor/download/printing/     F                                       Seems to work f
ine with Martin's oki4linux package.
Okidata OL 410e BW      A       LED     ljet4   PCL     600     600     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       
Okidata OL 600e BW      A       LED     ljet2p  PCL     300     300     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       
Okidata OL 610e/PS      BW      A       LED     N/A     PostScript      600     600     N/A
T                                       Native PS2 interpreter.
Okidata OL 610e/S       BW      D       Laser   laserjet        PCL     600     600     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Only does 300dp
i via PCL.
Okidata OL 800  BW      A       LED     laserjet        PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Printer emulates HP Las
erjet II,\nI used the laserjet GS driver \nand it works great.
Okidata OL 810e/PS      BW      A       LED     N/A     PostScript      600     600     N/A
T                                       Also speaks PCL 5e.\n<br>PCL-only model 810e should wor
k with ljet4 gs driver.
Okidata OL400ex BW      A       LED     ljet2p  PCL     300     300     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       
Okidata OL810ex BW      A       LED     ljet4   PCL     600     600     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       
Okidata OL830Plus       BW      A       LED     N/A     PostScript      300     300     N/A
T                                       Also supports PCL 3/HP LaserJet II\nand Diablo 630
Okidata OkiPage 4w+     BW      B       Laser   oki4w   Proprietary     300     300     http://
www.picante.com/~gtaylor/download/printing/     T                                       
Okidata Okijet 2010     Color   F       Ink Jet N/A     Proprietary     600     600     N/A
T                                       Offers PCL 3 emulation for DOS apps in Windows DOS box.
\n<br>compatible with HP 500c in Windows DOS box.\n<br>dual head\n<br>(parallel port)
Okidata Okijet 2500     Color   F       Ink Jet N/A     Proprietary     1200    600     N/A
F                                       Spoofs PCL for DOS apps under Windows, but other than t
hat just won't work.
Okidata Okipage 10e     BW      A       LED     ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Setup on server using A
PSfilter v5?\nas a 600x600 LaserJet 4. Shared to\nWin98 clients using Oki drivers \nlocally. Wo
rks like a charm but is\nsensitive to differnt papers.
Okidata Okipage 12i     BW      A       LED     N/A     PostScript      600     600     N/A
T                                       std 4M RAM; you may want more.
Okidata Okipage 20DXn   BW      A       Laser   N/A     PostScript      600     600     N/A
T                                       12MB RAM, duplex mechanism, network iface\n<br>Works on
 IEEE cable.\n<br>Network interface works, but only on a 10Mb hub?
Okidata Okipage 4w      BW      B       LED     oki4w   Proprietary     600     600     http://
www.picante.com/~gtaylor/download/printing/     F       OKI DATA CORP   OKI OKIPAGE 4w  HIPERWI
NDOWS,OPEL      OKI OKIPAGE 4w  600 dpi "class"; smoothing of some sort.\n<br>A winprinter; the
 protocol was reverse engineered and implemented by Marcin Dalecki.\n<br>The oki4linux package 
requires a moderately fast machine.\n<br>Aside from the klunkiness of it all, the printer print
s to the full specs.
Okidata Okipage 6e      BW      A       LED     ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       requires IEEE-1284 cabl
e.\n<br>"600dpi class" printer.\n<br>Printer may randomly leave PCL mode; fix: prepend the PJL 
command "^[%-12345X@PJL ENTER LANGUAGE=PCL" at\nthe start of the job, where the square characte
r is an escape character\n(0x1b).
Okidata Okipage 6ex     BW      A       LED     ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
Okidata Okipage 6w      BW      A       LED     oki4w   Proprietary     600     600     http://
www.picante.com/~gtaylor/download/printing/     T                                       It seem
s to have the same firmware as Okipage 4w.
Okidata Okipage 8c      Color   A       LED     N/A     PostScript      600     600     N/A
T                                       Also speaks PCL 5c.\n<br>32MB standard; uses regular SI
MMS.
Okidata Okipage 8w      BW      F       LED     N/A     Proprietary     600     600     N/A
T                                       
Okidata Super 6e        BW      B       LED     ljet4   PCL     300     1200    http://www.cs.w
isc.edu/~ghost/printer.html     T               OKIDATA OKIPAGE 6e      ENHANCED PCL5,PJL,EPSON
FX,IBMPPR,HIPERWINDOWS  OKIDATA OKIPAGE 6e (HP4P)       prints 300x300 using ljet4.
Olivetti        JP350S  Color   A       Ink Jet laserjet        PCL     300     300     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       The manual says
 it's a deskjet plus compatible, but selecting that in ghostscript doesn't work use laserjet in
stead\n\nThere is a color cartridge available for this printer, but I don't have it so I don't 
know if it works. According to olivetti it's hp deskjet 500c compatible
Olivetti        JP450   Color   A       Ink Jet djet500 PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
Olivetti        PG 306  BW      A       Laser   laserjet        PCL     300     300     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       NEVER remove th
e little brass plate of the\ntoner drum unit - I've wrecked my printer by doing this !
PCPI    1030    BW      A       Laser   laserjet        PCL     300     300     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Works fine, although a 
little slow.\n<br>A Postscript option is available.
Panasonic       KX-P1123        BW      A       Dot Matrix      epson   ESC/P   120     180
http://www.cs.wisc.edu/~ghost/printer.html      T                                       
Panasonic       KX-P1124        BW      A       Dot Matrix      epson   ESC/P           
http://www.cs.wisc.edu/~ghost/printer.html      T                                       
Panasonic       KX-P1150        BW      A       Dot Matrix      eps9high        ESC/P   240
216     http://www.cs.wisc.edu/~ghost/printer.html      T                               
9 pin printer; try various eps9* drivers.
Panasonic       KX-P2023        BW      A       Dot Matrix      epson   Proprietary     360
360     http://www.cs.wisc.edu/~ghost/printer.html      F                               
It will only print the left 2/3 of a page if you have\nthe text printing mode set to 16cpi. Bes
t results\nif you leave the poweron modes to default and make\nsure it is using the epson codes
et.
Panasonic       KX-P2123        Color   B       Dot Matrix      epson   ESC/P   360     360
http://www.cs.wisc.edu/~ghost/printer.html      T                                       
Panasonic       KX-P2135        Color   A       Dot Matrix      epsonc  ESC/P   360     180
http://www.cs.wisc.edu/~ghost/printer.html      T                                       Works f
ine in color and b&w with \nghostscript, magicfilter epsonlqc \nsetting. Capable of 360x180.\nV
ery quiet
Panasonic       KX-P2150        BW      A       Dot Matrix      ibmpro  ASCII   144     240
http://www.cs.wisc.edu/~ghost/printer.html      T                                       <p>aka 
<i>PanaPrinter 192</i>; 9-pin printer; emulates Epson FX-86e/FX-800 (standard mode)or IBM ProPr
inter II (IBM mode, which is not used under Linux)  \n\n<p>Works <b>perfectly</b> using <tt>ibm
pro</tt> GS driver in standard mode.  Red Hat Linux does not support <tt>ibmpro</tt> in printto
ol and rhs-printfilters components.  To use <tt>ibmpro</tt> GS driver in RH, uninstall those tw
o components and install 3rd party print manager like APSFilter.\n\n<p>Works <b>mostly</b> usin
g <tt>epson</tt> GS driver in standard mode.  Alignment problems exist for printing lines after
 attempting to print graphics, tables, figures, and/or equations in Postscript documents (using
 Ghostscript).  \n\n<p>Works <b>perfectly</b> using <tt>epson</tt> driver if Postscript documen
t contains only text or document is raw ASCII.\n\n<p>
Panasonic       KX-P4410        BW      A       Laser   ljet2p  PCL     300     300     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
Panasonic       KX-P4450        BW      A       Laser   ljet3   PCL     300     300     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       1M RAM std, 5M 
max.
Panasonic       KX-P5400        BW      A       LED     N/A     PostScript      300     300
N/A     T                                       
Panasonic       KX-P6100        BW      F       Laser   N/A     Proprietary     1200    300
N/A     T                                       "Supports PCL for DOS apps in Windows DOS box"\
n<br>...so I assume this is a paperweight.
Panasonic       KX-P6150        BW      B       Laser   laserjet        PCL     600     300
http://www.cs.wisc.edu/~ghost/printer.html      T                                       My firs
t foray into Linux but I got\nthis printer to work just about!  I\nused the printer setup from 
control\npanel - ASCII prints fine, but there\nare currently no KXP drivers/filters\nso you hav
e to use the HP Laserjet\none.  It doesn't work perfectly, the\nbottom of the page was cut off 
but\nI'm sure someone with a better\nunderstanding of these things could\nfix it easily enough.
Panasonic       KX-P6300 GDI    BW      F       Laser   N/A     Proprietary             
N/A     F                                       I can't really find a good specs page for this 
printer, but it appears to be a "GDI" printer.\n<br>It seems to use the same Windows driver as 
the 6500?\n<br>The Japan Panasonic page has the most info, with various resease notes etc.
Panasonic       KX-P6500        BW      D       Laser   ljet2p  PCL     2400    600     http://
www.cs.wisc.edu/~ghost/printer.html     T       Panasonic       KX-P6500        GDI     (no suc
h line for this device) Printing at the full resolution requires the Windows driver.\n<br>Ghost
script drives it at 300dpi.\n<br>Base memory is 512K; it is very slow.  Seems to take regular S
IMMs, 4M should help.\n<br>This is a cheap printer; operating costs surpass the cost of the pri
nter before 10k pages!\n<br>The maximum 2400x600 mode is probably just 600x600 with smoothing.
Panasonic       KX-P8410        Color   F       Laser   N/A     Proprietary     1200    1200
N/A     T                                       Requires more than std 8M RAM for 1200dpi.\nSup
ports upto 72megs RAM\n<br>No hint of non-Windows support.\nUses Panasonic GDI format.\nSupport
s both SCSI2 and Parallel input.
Panasonic       KX-P8420        Color   A       Laser   N/A     PostScript      1200    1200
N/A     T                                       80MB std; uses regular DIMMs.
Panasonic       KX-P8475        Color   A       Laser   N/A     PostScript      1200    1200
N/A     T                                       Also PCL5.\n<br>8M RAM std, full resolution req
uires 24M.\n<br>SCSI interface\n<br>various network options
Printrex        820 DL  BW      D       Thermal N/A     Proprietary     200     200     N/A
T                                       Data logging printer; prints on continuous roll thermal
 paper.\n<br>Prints text just fine, and the command set is fully documented.\n<br>It should be 
straightforward to write a Ghostscript driver for the\ntwo bitmap modes (100 and 200dpi).
QMS     2425 Turbo EX   BW      A       Laser   N/A     PostScript      1200    1200    N/A
T                                       
QMS     ps-810  BW      B       Laser   N/A     PostScript      300     300     N/A     F
                                Very old (circa '89), reliable printer.\nLaserwriter clone.\nNa
tive postscript level 1.\napsfilter handles ascii etc.\nprint from netscape works.\nprint from 
acrobat reader does not,\neven w/level 1 set.
Ricoh   4081    BW      A       Laser   r4081   Proprietary                     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
Ricoh   4801    BW      A       Laser   r4081   Proprietary                     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
Ricoh   6000    BW      A       Laser   r4081   Proprietary                     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       
Ricoh   Aficio 401      BW      B       Thermal transfer        ljet4   PCL     400     400
http://www.cs.wisc.edu/~ghost/printer.html      T                                       I used 
the existing magicfilter filter\nfor HP LaserJet 4, but changed resolution\nfrom 600 to 300.\nI
 have only made it print in 300dpi,\nand the marigins need adjustment.\nPrinting mechanism is s
pecified as\n"Dry Electrostatic Transfer System"
Ricoh   Aficio AP2000   BW      A       Laser   N/A     PostScript      1200    1200    N/A
T                                       Accepts A3 paper - Network card (optional) comes with l
istening port set to default as 10000 - change to 514 - can be configred via telnet or httpd
Ricoh   Aficio Color 2206       Color   F       Laser   N/A     PostScript      600     600
N/A     F                                       This printer is driven by a Fiery XJ80e print s
erver. It's a network printer, and it supports most protocols (IP, AppleTalk, Novell, etc.). Th
e machine has a local parallel port and built in 10Base-T. I can't get it to work through 10Bas
e-T, and it's too big to move for trying the parallel port.
Ricoh   Afico FX10      BW      F       Laser   N/A     Proprietary     400     400     N/A
F                                       Scanner/fax/copier/printer.\n<br>Web site says PCL 4.5 
in Windows DOS box; I assume this means no onboard PCL.\n<br>Scanner also not known to work.
Samsung ML-5000a        BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       This printer is a true 
PCL printer(level 5e). with ljet4 drivers prints excellent
Samsung ML-5050G        BW      B       Laser   N/A     PCL     600     600     N/A     T
                                This printer though designed to \nbe a Windows 95 printer can b
e tweeked to function usgin HP4 or HP5 PCL drivers under Linux
Samsung ML-6000/6100    BW      A       Laser   N/A     PCL     600     600     N/A     F
                                The 6000 and 6100 are PCL5 printers and will work unsing HP dri
vers without a problem.
Samsung ML-7000/7000P/7000N     BW      A       Laser   N/A     PCL     1200    1200    N/A
F                                       This is a true blood PCL6 Printer\nand will function co
mpletly with PCL 5 drivers, though the "print on both side of paper" control need to be issued 
from teh printer control panel and will not work through the software in Linux
Samsung ML-7050 BW      A       Laser   N/A     PCL     1200    1200    N/A     F       
                        This is a Network version of the ML-7000 printer by samsung, and will w
ork with HP5 drivers.
Samsung ML-85   BW      A       LED     lj4dith PCL     600     600     http://www.cs.wisc.edu/
~ghost/printer.html     F                                       This printer is a Proper HP4/HP
5 PCL compatible printer.
Samsung ML-85G  BW      F       Laser   N/A     Proprietary     600     600     N/A     F
                                "Windows Printing System", with software-based PCL emulation.\n
<br>OTOH, the specs claim PCL 4 as "standard".  Hmm...
Samsung SF/MSYS/MJ-4700/4800/4500C      Color   F       Ink Jet N/A     Proprietary     1200
1200    N/A     F                                       These are GDI driven printers devise to
 work under Windows (w2k drivers still under construction) and there are no plans to make this 
Multifunction machine work under Linux.
Seiko   SLP     BW      B       Thermal slap    Proprietary                     http://members.
tripod.com/~uutil/slap/ F                                       Not a postscript or pcl driver;
 slap only prints text in various fonts.
Seiko   SLP 120 BW      B       Thermal slap    Proprietary                     http://members.
tripod.com/~uutil/slap/ F                                       Not a postscript or pcl driver;
 slap only prints text in various fonts.
Seiko   SLP 220 BW      B       Thermal slap    Proprietary                     http://members.
tripod.com/~uutil/slap/ F                                       Not a postscript or pcl driver;
 slap only prints text in various fonts.
Seiko   SLP EZ30        BW      B       Thermal slap    Proprietary                     http://
members.tripod.com/~uutil/slap/ F                                       Not a postscript or pcl
 driver; slap only prints text in various fonts.
Seiko   SLP Plus        BW      B       Thermal slap    Proprietary                     http://
members.tripod.com/~uutil/slap/ F                                       Not a postscript or pcl
 driver; slap only prints text in various fonts.
Seiko   SLP Pro BW      B       Thermal slap    Proprietary                     http://members.
tripod.com/~uutil/slap/ F                                       Not a postscript or pcl driver;
 slap only prints text in various fonts.
Seiko   SpeedJET 200    BW      A       Ink Jet laserjet        PCL     300     300     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Another Olivett
i JP350 clone.
Sharp   AR-161  BW      A       Laser   ljet4   PCL     600     600     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       Digital copier; printer functio
nality is option AR-PB8.\n<br>Postscript option available.\n<br>Network option avaialble.\n<br>
Fax option available.\n<br>16 ppm; 8MB std(?), but takes regular DIMMs to 136MB.
Star    LC 90   BW      B       Dot Matrix      epson   ESC/P                   http://www.cs.w
isc.edu/~ghost/printer.html     T                                       This printer print, wit
h ghostscript.\nBut its designed for resolution \n240x144 dpi. So problem is, by using\nresolut
ion 240x180 dpi, that text is\nsmaller and it is not used all place\n on the A4 format paper.
Star    LC24-100        BW      A       Dot Matrix      necp6   ESC/P   360     360     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       <P>It may be ne
cessary to turn the\nprinter off and on when switching\nresolution (I only tried high\nresoluti
on once, but I have used\n360x180 for years).</P>\n<P>This is an old but durable\nprinter, I ha
ve used it for 5 years\nwith no problems.</P>\n<P>The printer is no\nlonger being sold, but the
 manual\nthat shipped with it contains full\nprogramming documentation of all\nprinter features
 making it possible\nto write your own drivers for any\nOS or application.</P>
Star    LC24-200        Color   B       Dot Matrix      epson   ESC/P                   http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Partial report 
from Leonid Mamtchenko.\n<br>BW printing with Ghostscript works OK, although it's a bit blurry.
\n<br>He hasn't tried color printing yet.\n<br>I don't know which gs device he's using.<br>\n<p
><div align=justify>\nReport from Dominik Mierzejewski <dmierzej@elka.pw.edu.pl>:<br>\nI've con
figured it using RedHat's Printtool and it seems\nto work perfectly, although I haven't tried m
y colour\nribbon yet. Both Ghostscript and ascii printing is ok.\nThe driver is listed as 'Epso
n Dot Matrix, 24 pin', which is\nprobably 'epson' for GS.\n</div></p>
Star    NL-10   BW      A       Dot Matrix      epson   ESC/P   240     216     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Alternate drivers: eps9
mid, eps9high\n<br>Margin adjustments may need some tuning.
Star    StarJet 48      BW      B       Ink Jet sj48    Proprietary     360     360     http://
www.cs.wisc.edu/~ghost/printer.html     F                                       
Star    WinType 4000    BW      F       Laser   N/A     Proprietary     600     600     N/A
F                                       WinPrinter - Uses GDI interface, supports PCL and OS in
 software mode only
Tally   MT908   BW      A       Laser   ljet3   PCL     300     300     http://www.cs.wisc.edu/
~ghost/printer.html     T                                       Works in lj3 emulation mode; th
e printer also has proprinter and epson modes.
Tektronix       3693d color printer, 8-bit mode Color   A       Dot Matrix      t4693d8 Proprie
tary    100     100     http://www.cs.wisc.edu/~ghost/printer.html      T               
                
Tektronix       4693d color printer, 2-bit mode Color   A       Dot Matrix      t4693d2 Proprie
tary    100     100     http://www.cs.wisc.edu/~ghost/printer.html      T               
                
Tektronix       4693d color printer, 4-bit mode BW      A       Dot Matrix      t4693d4 Proprie
tary    100     100     http://www.cs.wisc.edu/~ghost/printer.html      T               
                
Tektronix       4695    BW      A       Ink Jet tek4696 Proprietary     120     120     http://
www.cs.wisc.edu/~ghost/printer.html     F                                       
Tektronix       4696    Color   A       Ink Jet tek4696 Proprietary     180     180     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
Tektronix       4697    Color   A       Ink Jet tek4696 Proprietary     216     216     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Just change the
 resolution in the\n4696 driver
Tektronix       Phaser 780      Color   A       Laser   N/A     PostScript      1200    1200
N/A     F                                       used RedHat's "printtool"; set to "postscript,"
 color test page printed perfectly (in fact, stunningly).
Tektronix       Phaser IISX     Color   A       Thermal N/A     PostScript      300     300
N/A     F                                       20 meg of ram, scsi disk.\nmuch faster than PX
Tektronix       Phaser PX       Color   A       Thermal N/A     PostScript      300     300
N/A     F                                       Wax Depositon Postscript Printer\n12 megs of ra
m, 68k processor\nvery, very slow.
Xerox   2700 XES        BW      A       Ink Jet xes     Proprietary     300     300     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
Xerox   3700 XES        BW      A       Ink Jet xes     Proprietary     300     300     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
Xerox   4045 XES        BW      A       Ink Jet xes     Proprietary     300     300     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       
Xerox   DocuPrint 4508  BW      A       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Had trouble printing va
rious\nsample files from gs distribution\nbut adding 4 Mb RAM solved it.
Xerox   DocuPrint C55   Color   A       Laser   N/A     PostScript      600     600     N/A
T                                       Also supports PCL5c.<br>\nPrinting with Generic PS file
ter does not support\nfull printer functionality.
Xerox   DocuPrint N17   BW      A       Laser   N/A     PostScript      1200    600     N/A
T                                       Also supports PCL5e.<br>\nWill print well with generic 
PS drivers. \nFull printer functionality is not supported through the generic filters.
Xerox   DocuPrint N32   BW      A       Laser   N/A     PostScript      600     600     N/A
T                                       Printer also supports PCL5e. \nGeneric PS filters do no
t support full printer functionality of stapling,\nmedia type support, or output destinations.<
br>\nA wonderful workhorse of a printer.
Xerox   DocuPrint P12   BW      B       Laser   ljet2p  PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       Another "PrintGear" pri
nter; PCL support only goes up to 300dpi.\n<br>There is a $100 rebate coupon on the Xerox websi
te.
Xerox   DocuPrint P8    BW      F       Laser   N/A     Proprietary                     N/A
F                                       This is an entirely different printer from the P8<b>e</
b>.\n<br>This printer is a "winprinter"; the P8<b>e</b> is a supported PCL printer.
Xerox   DocuPrint P8e   BW      B       Laser   ljet4   PCL     600     600     http://www.cs.w
isc.edu/~ghost/printer.html     T                                       This printer is not to 
be confused with the P8 (no <b>e</b>).\n<br>Danger Will Robinson: Despite this being a PCL prin
ter, \nthere is a slight annoyance. When the printer goes into \npower saver mode, any Windows-
based access over a network \nrequires that the printer be "woken up" before any serious \nprin
ting can be done. This is, I think, because the LaserJet \n4 doesn't have a "sleep" mode. Anywa
y, give it a kick with a \ntext-based editor (like PFE from Windows) and <I>then</I> it \nworks
 fine. Accessing the controls for the printer to turn \nPower Save mode off permanently can onl
y be done from Windows,\nAFAIK so I'm still looking for a way around this.\nAlso, toner cartrid
ges are <I>very</I> expensive (3 carts == 1 P8e)!
Xerox   Document Homecentre     Color   B       Ink Jet cdj550  PCL     600     600     http://
www.cs.wisc.edu/~ghost/printer.html     T       Xerox   Document HomeCentre     none    
Multifunction Printer/Scanner\nHave not explored scanner functionality under Linux\nWorks quite
 well as a 300x300 dpi \ncolor printer. \nPrinter Doc's claim 600x600 on \nspecial paper, but 3
00x300 on normal\npaper looks good.
Xerox   WorkCentre 450cp        Color   D       Ink Jet cdj550  PCL     600     600     http://
www.cs.wisc.edu/~ghost/printer.html     T                                       Reported to wor
k with cdj550 driver, which may be limited to 300dpi?\n<br>Multifunction device; no Linux suppo
rt for other features.
Xerox   WorkCentre XD120f       BW      F       Laser   N/A     Proprietary             
N/A     F                                       Another WinPrinter
Xerox   XJ6C    Color   B       Ink Jet N/A     PCL     1200    600     N/A     T       
                        I've had luck using the\ncolor deskjet drivers although\nthey don't sup
port the\nprinter's high resolutions.
Xerox   XJ8C    BW      D       Ink Jet lxm5700m        Proprietary     1200    1200    http://
www.ultranet.com/~setaylor/papers.htm   F                                       Works with driv
er for Lexmark 5700.

#section: drivers
#format: make\tmodel\tresolution_x\tresolution_y\tcolor\tcmd_type_short\tcmd\tcomment
Apple   12/640ps        600     600     BW      Postscript              
Apple   Dot Matrix      120     72      BW      Ghostscript     -sDEVICE=appledmp       
Apple   ImageWriter LQ  320     216     BW      Ghostscript     -sDEVICE=iwlq   
Apple   ImageWriter high-res    120     144     BW      Ghostscript     -sDEVICE=iwhi   
Apple   ImageWriter low-res     120     72      BW      Ghostscript     -sDEVICE=iwlo   
Apple   LaserWriter 16/600      600     600     BW      Postscript              
Apple   LaserWriter IINTX       300     300     BW      Postscript              Can do ascii, H
P Laserjet+, or Diablo 620, by sending special PostScript, or switch setting.
Apple   LaserWriter Select 360  300     300     BW      Ghostscript     -sDEVICE=ljet2p 
Apple   StyleWriter 2500        360     360     Color   Ghostscript     -sDEVICE=bitcmyk -r360x
360 -g3060x3960 Color printing mode
Apple   StyleWriter 2500        360     360     Color   Postscript      |/usr/sbin/lpstyl -t bi
tcmyk -w 3060 -h 3960   Ghostscript output is piped to lpstyl, available from RPM sites
Brother HJ-400  360     360     BW      Ghostscript     -sDEVICE=lq850  Not included in the Red
Hat ghostscript
Brother HJ-400  360     360     BW      Ghostscript     -sDEVICE=lq850  
Brother HL-1040 300     300     BW      Ghostscript     -sDEVICE=ljet2p -r300x300       
Brother HL-1050 600     600     BW      Ghostscript     -sDEVICE=ljet4 -r600x600        
Brother HL-1060 600     600     BW      Ghostscript     -sDEVICE=ljet4 -r600x600        
Brother HL-1060 600     600     BW      Postscript              with postscript option; slow
Brother HL-10V  300     300     BW      Ghostscript     -sDEVICE=ljet3 -r300x300        
Brother HL-1250 600     600     BW      Ghostscript     -sDEVICE=ljet4 -r600x600        
Brother HL-1260 600     600     BW      Postscript              
Brother HL-1260 600     600     BW      Ghostscript     -sDEVICE=ljet4 -r600x600        using p
cl emulation
Brother HL-2060 1200    1200    BW      Postscript              
Brother HL-630  150     150     BW      Ghostscript     -sDEVICE=ljet2p -r150x150       should 
fit in std 0.5MB memory
Brother HL-630  300     300     BW      Ghostscript     -sDEVICE=ljet2p probably requires 2MB m
emory
Brother HL-660  600     600     BW      Ghostscript     -sDEVICE=ljet4 -r600x600        
Brother HL-660  600     600     BW      Postscript              with postscript option
Brother HL-720  300     300     BW      Ghostscript     -sDEVICE=hl7x0 -r300x300        
Brother HL-720  600     600     BW      Ghostscript     -sDEVICE=hl7x0  
Brother HL-730  150     150     BW      Ghostscript     -sDEVICE=hl7x0 -r150x150        
Brother HL-730  300     300     BW      Ghostscript     -sDEVICE=hl7x0 -r300x300        
Brother HL-730  600     600     BW      Ghostscript     -sDEVICE=hl7x0 -r600x600        
Brother HL-730  600     600     BW      Ghostscript     -sDEVICE=hl7x0 -600x600 
Brother HL-760  150     150     BW      Ghostscript     -sDEVICE=ljet4 -r150x150        uses le
ss printer memory
Brother HL-760  300     300     BW      Ghostscript     -sDEVICE=ljet4 -r300x300        
Brother HL-760  600     600     BW      Ghostscript     -sDEVICE=ljet4 -r600x600        
Brother HL-8    150     150     BW      Ghostscript     -sDEVICE=ljetplus -r150x150     works w
/ standard 512K RAM
Brother HL-8    300     300     BW      Ghostscript     -sDEVICE=ljetplus       requires 1M (or
 more?)
Brother HL-820  600     600     BW      Ghostscript     -sDEVICE=hl7x0  
Brother MC-3000 360     180     BW      Ghostscript     -sDEVICE=epson -r360x180        
Canon   BJ-10e  360     360     BW      Ghostscript     -sDEVICE=bj10e  
Canon   BJ-20   360     360     BW      Ghostscript     -sDEVICE=bj10e  
Canon   BJ-200  360     360     BW      Ghostscript     -sDEVICE=bj200  
Canon   BJC-1000        720     360     Color   Postscript              
Canon   BJC-2000        360     360     Color   Ghostscript     -sDEVICE=bjc600 -r360   Standar
d 360x360 Colour print mode
Canon   BJC-2000        360     360     Color   Ghostscript     -sDEVICE=bjc600 -sPrintQuality=
Draft -dMonochromePrint=false   Very Ugly Colour Draft Mode
Canon   BJC-2000        720     360     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-210 360     360     Color   Ghostscript     -sDEVICE=bjc600 -r360x360       
Canon   BJC-210 360     360     BW      Ghostscript     -sDEVICE=bj200 -r360x360        
Canon   BJC-210 720     360     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-240 360     360     Color   Postscript      -sDEVICE=bjc600 -r360x360       
Canon   BJC-240 360     360     BW      Ghostscript     -sDEVICE=bj200 -r360x360        
Canon   BJC-240 360     360     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-250 360     360     Color   Ghostscript     @bjc610a0.upp   fast on plain paper
Canon   BJC-250 360     360     Color   Ghostscript     @bjc610a1.upp   for plain paper
Canon   BJC-250 360     360     Color   Ghostscript     @bjc610a2.upp   for coated paper
Canon   BJC-250 360     360     Color   Ghostscript     @bjc610a3.upp   for transparency film
Canon   BJC-250 360     360     Color   Ghostscript     @bjc610a4.upp   for back print film
Canon   BJC-250 360     360     Color   Ghostscript     @bjc610a5.upp   for fabric sheet
Canon   BJC-250 360     360     Color   Ghostscript     @bjc610a6.upp   for glossy paper
Canon   BJC-250 360     360     Color   Ghostscript     @bjc610a7.upp   for high gloss film
Canon   BJC-250 360     360     Color   Ghostscript     @bjc610a8.upp   for high resolution pap
er
Canon   BJC-250 360     360     Color   Ghostscript     -sDEVICE=bj200 -r360x360        
Canon   BJC-4000        360     360     BW      Ghostscript     -sDEVICE=bjc600 -sProcessColorM
odel=DeviceGray -dBitsPerPixel=1        
Canon   BJC-4000        360     360     BW      Ghostscript     -sDEVICE=bjc600 -sProcessColorM
odel=DeviceGray -dBitsPerPixel=8        dithered; better greyscale
Canon   BJC-4000        360     360     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-4100        360     360     BW      Ghostscript     -sDEVICE=bjc600 -sProcessColorM
odel=DeviceGray -dBitsPerPixel=1        
Canon   BJC-4100        360     360     BW      Ghostscript     -sDEVICE=bjc600 -sProcessColorM
odel=DeviceGray -dBitsPerPixel=8        dithered; better greyscale
Canon   BJC-4100        360     360     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-4200        360     360     BW      Ghostscript     -sDEVICE=bjc600 -sProcessColorM
odel=DeviceGray -dBitsPerPixel=1        
Canon   BJC-4200        360     360     BW      Ghostscript     -sDEVICE=bjc600 -sProcessColorM
odel=DeviceGray -dBitsPerPixel=8        dithered; better greyscale
Canon   BJC-4200        360     360     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-4300        360     360     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-4300        360     360     BW      Ghostscript     -sDEVICE=bjc600 -sProcessColorM
odel=DeviceGray -dBitsPerPixel=1        
Canon   BJC-4300        360     360     BW      Ghostscript     -sDEVICE=bjc600 -sProcessColorM
odel=DeviceGray -dBitsPerPixel=8        dithered; better greyscale
Canon   BJC-4310SP      360     360     BW      Ghostscript     -sDEVICE=bjc600 -sProcessColorM
odel=DeviceGray -dBitsPerPixel=1        
Canon   BJC-4310SP      360     360     BW      Ghostscript     -sDEVICE=bjc600 -sProcessColorM
odel=DeviceGray -dBitsPerPixel=8        dithered; better greyscale
Canon   BJC-4310SP      360     360     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-4400        360     360     BW      Ghostscript     -sDEVICE=bjc600 -sProcessColorM
odel=DeviceGray -dBitsPerPixel=1        
Canon   BJC-4400        360     360     BW      Ghostscript     -sDEVICE=bjc600 -sProcessColorM
odel=DeviceGray -dBitsPerPixel=8        dithered; better greyscale
Canon   BJC-4400        360     360     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-4550        360     360     Color   Ghostscript     -sDEVICE=bjc600 -r360x360

Canon   BJC-5100        1440    720     Color   Ghostscript             
Canon   BJC-600 360     360     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-6000        360     360     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-6000        360     360     BW      Ghostscript     -sDEVICE=bjc600 -dProcessColorM
odel=DeviceGray 
Canon   BJC-6000        1440    720     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-610 360     360     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-610 360     360     BW      Ghostscript     -sDEVICE=bjc600 -dProcessColorModel=Dev
iceGray 
Canon   BJC-610 360     360     Color   Ghostscript     @bjc610a0.upp   fast on plain paper
Canon   BJC-610 360     360     Color   Ghostscript     @bjc610a1.upp   for plain paper
Canon   BJC-610 360     360     Color   Ghostscript     @bjc610a2.upp   for coated paper
Canon   BJC-610 360     360     Color   Ghostscript     @bjc610a3.upp   for transparency film
Canon   BJC-610 360     360     Color   Ghostscript     @bjc610a4.upp   for back print film
Canon   BJC-610 360     360     Color   Ghostscript     @bjc610a5.upp   for fabric sheet
Canon   BJC-610 360     360     Color   Ghostscript     @bjc610a6.upp   for glossy paper
Canon   BJC-610 360     360     Color   Ghostscript     @bjc610a7.upp   for high gloss film
Canon   BJC-610 360     360     Color   Ghostscript     @bjc610a8.upp   for high resolution pap
er
Canon   BJC-610 360     360     BW      Ghostscript     -sDEVICE=bj200 -r360x360        
Canon   BJC-610 720     720     Color   Ghostscript     @bjc610b0.upp   fast on plain paper
Canon   BJC-610 720     720     Color   Ghostscript     @bjc610b1.upp   for plain paper
Canon   BJC-610 720     720     Color   Ghostscript     @bjc610b2.upp   for coated paper
Canon   BJC-610 720     720     Color   Ghostscript     @bjc610b3.upp   for transparency film
Canon   BJC-610 720     720     Color   Ghostscript     @bjc610b4.upp   for back print film
Canon   BJC-610 720     720     Color   Ghostscript     @bjc610b5.upp   for fabric sheet
Canon   BJC-610 720     720     Color   Ghostscript     @bjc610b6.upp   for glossy paper
Canon   BJC-610 720     720     Color   Ghostscript     @bjc610b7.upp   for high gloss film
Canon   BJC-610 720     720     Color   Ghostscript     @bjc610b8.upp   for high resolution pap
er
Canon   BJC-620 360     360     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-620 360     360     BW      Ghostscript     -sDEVICE=bjc600 -dProcessColorModel=Dev
iceGray 
Canon   BJC-620 360     360     Color   Ghostscript     @bjc610a0.upp   fast on plain paper
Canon   BJC-620 360     360     Color   Ghostscript     @bjc610a1.upp   for plain paper
Canon   BJC-620 360     360     Color   Ghostscript     @bjc610a2.upp   for coated paper
Canon   BJC-620 360     360     Color   Ghostscript     @bjc610a3.upp   for transparency film
Canon   BJC-620 360     360     Color   Ghostscript     @bjc610a4.upp   for back print film
Canon   BJC-620 360     360     Color   Ghostscript     @bjc610a5.upp   for fabric sheet
Canon   BJC-620 360     360     Color   Ghostscript     @bjc610a6.upp   for glossy paper
Canon   BJC-620 360     360     Color   Ghostscript     @bjc610a7.upp   for high gloss film
Canon   BJC-620 360     360     Color   Ghostscript     @bjc610a8.upp   for high resolution pap
er
Canon   BJC-620 360     360     BW      Ghostscript     -sDEVICE=bj200 -r360x360        
Canon   BJC-620 720     720     Color   Ghostscript     @bjc610b0.upp   fast on plain paper
Canon   BJC-620 720     720     Color   Ghostscript     @bjc610b1.upp   for plain paper
Canon   BJC-620 720     720     Color   Ghostscript     @bjc610b2.upp   for coated paper
Canon   BJC-620 720     720     Color   Ghostscript     @bjc610b3.upp   for transparency film
Canon   BJC-620 720     720     Color   Ghostscript     @bjc610b4.upp   for back print film
Canon   BJC-620 720     720     Color   Ghostscript     @bjc610b5.upp   for fabric sheet
Canon   BJC-620 720     720     Color   Ghostscript     @bjc610b6.upp   for glossy paper
Canon   BJC-620 720     720     Color   Ghostscript     @bjc610b7.upp   for high gloss film
Canon   BJC-620 720     720     Color   Ghostscript     @bjc610b8.upp   for high resolution pap
er
Canon   BJC-620 720     720     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-70  360     360     Color   Ghostscript     -sDEVICE=bjc600 
Canon   BJC-70  360     360     BW      Ghostscript     -sDEVICE=bj20   
Canon   BJC-7000        600     600     Color   Postscript      -sDEVICE=bjc800 
Canon   BJC-7000        1200    600     Color   Ghostscript     -sDEVICE=bjc800 
Canon   BJC-7004        600     600     Color   Ghostscript     -sDEVICE=bjc800 
Canon   BJC-7100        1200    600     Color   Ghostscript     -sDEVICE=bjc800 very slow
Canon   BJC-800 720     720     Color   Ghostscript     -sDEVICE=bjc800 -r720x720       
Canon   LBP-1260        600     600     BW      Ghostscript     -sDEVICE=ljet4 -r600x600
may need more memory; works w/7MB
Canon   LBP-4U  300     300     BW      Ghostscript     -sDEVICE=lbp8   page is offset funny
Canon   LBP-4U  300     300     BW      Postscript      gs -sDEVICE=ljet3 -r300x300 -q -dSAFER 
-dNOPAUSE -sOutputFile=- | cjet -q      
Canon   LBP-4sx 300     300     BW      Ghostscript     -sDEVICE=ljet3  
Canon   LIPS III        300     300     BW      Ghostscript     -sDEVICE=lips3  
Canon   MultiPASS C2500 360     360     Color   Ghostscript     -sDEVICE=bjc600 -r360x360

Canon   MultiPASS C3500 360     360     Color   Ghostscript     -sDEVICE=bjc600 -r360x360

Canon   MultiPASS C5000 720     360     Color   Ghostscript     -sDEVICE=bjc600 
DEC     DECWriter 500i  300     300     BW      Ghostscript     -sDEVICE=djet500        
DEC     DECwriter 110i  300     300     BW      Ghostscript     -sDEVICE=djet500        
DEC     DECwriter 520ic 300     300     Color   Ghostscript     -sDEVICE=cdj500 
DEC     LA50    144     72      BW      Ghostscript     -sDEVICE=la50   
DEC     LA75    144     72      BW      Ghostscript     -sDEVICE=la75   
DEC     LA75 Plus       180     180     BW      Ghostscript     -sDEVICE=la75plus       
DEC     LN03    300     300     BW      Ghostscript     -sDEVICE=ln03   
Epson   9 Pin Printers high-res 240     216     BW      Ghostscript     -sDEVICE=eps9high

Epson   9 Pin Printers med-res  240     216     BW      Ghostscript     -sDEVICE=eps9mid

Epson   AP3250  360     360     BW      Ghostscript     -sDEVICE=ap3250 
Epson   EPL 5700        300     300     BW      Ghostscript     -sDEVICE=ljet4  
Epson   LQ-24   240     216     Color   Ghostscript     -sDEVICE=epsonc 
Epson   LQ-2550 60      72      Color   Ghostscript     -sDEVICE=epsonc 
Epson   LQ-2550 60      60      Color   Ghostscript     -sDEVICE=epsonc 
Epson   LQ-2550 120     72      Color   Ghostscript     -sDEVICE=epsonc 
Epson   LQ-2550 120     60      Color   Ghostscript     -sDEVICE=epsonc 
Epson   LQ-2550 240     72      Color   Ghostscript     -sDEVICE=epsonc 
Epson   LQ-2550 240     60      Color   Ghostscript     -sDEVICE=epsonc 
Epson   Stylus Color    720     720     Color   Ghostscript     @       
Epson   Stylus Color 1520       360     360     Color   Ghostscript     -sDEVICE=stcolor

Epson   Stylus Color 1520       1440    720     Color   Ghostscript     @stc1520h       
Epson   Stylus Color 300        360     360     Color   Postscript      @       
Epson   Stylus Color 400        720     720     Color   Ghostscript     -sDEVICE=stcolor

Epson   Stylus Color 400        720     720     Color   Postscript      -sDEVICE=stcolor

Epson   Stylus Color 440        360     720     Color   Ghostscript     -sDEVICE=stcolor

Epson   Stylus Color 440        720     720     Color   Ghostscript     -sDEVICE=stcolor

Epson   Stylus Color 500        360     360     Color   Ghostscript     @stc500p        
Epson   Stylus Color 500        720     720     Color   Ghostscript     @stc500ph       
Epson   Stylus Color 600        360     360     Color   Ghostscript     @stc600pl       
Epson   Stylus Color 600        720     720     Color   Ghostscript     @stc600p        
Epson   Stylus Color 600        1440    720     Color   Ghostscript     @stc600ih       
Epson   Stylus Color 640        360     360     Color   Ghostscript     @stc600pl.upp   Fast, f
or plain paper
Epson   Stylus Color 640        720     720     Color   Ghostscript     @stc600p.upp    Medium 
quality, plain paper
Epson   Stylus Color 640        1440    720     Color   Ghostscript     -sDEVICE=stcolor

Epson   Stylus Color 640        1440    720     Color   Ghostscript     @stc600ih.upp   High qu
ality, inkjet paper
Epson   Stylus Color 640        1440    720     Color   Ghostscript     -sDEVICE=stcolor

Epson   Stylus Color 660        1440    720     Color   Ghostscript     @stc600.upp -dMicroweav
e       
Epson   Stylus Color 740        720     1440    Color   Ghostscript     @       
Epson   Stylus Color 740        720     1440    Color   Postscript      @stc740 
Epson   Stylus Color 900        360     360     Color   Ghostscript     @stc500p.upp    not wea
ved
Epson   Stylus Color 900        720     720     Color   Ghostscript     @stc500ph.upp   not wea
ved
Epson   Stylus Color I  360     360     Color   Ghostscript     @stc.upp        
Epson   Stylus Color I  360     360     Color   Ghostscript     @stc_l.upp      
Epson   Stylus Color I  720     720     Color   Ghostscript     @stc_h.upp      
Epson   Stylus Color PRO        360     360     Color   Ghostscript     @stc.upp        
Epson   Stylus Color PRO        360     360     Color   Ghostscript     @stc_l.upp      
Epson   Stylus Color PRO        720     720     Color   Ghostscript     @stc_h.upp      
Fujitsu 1200    240     72      Color   Ghostscript     -sDEVICE=epsonc 
Fujitsu 2400    240     72      Color   Ghostscript     -sDEVICE=epsonc 
Fujitsu 3400    240     72      Color   Ghostscript     -sDEVICE=epsonc 
Fujitsu PrintPartner 10V        600     600     BW      Ghostscript     -sDEVICE=ljet4 -r600

Fujitsu PrintPartner 16DV       600     600     BW      Ghostscript     -sDEVICE=ljet4 -r600

Fujitsu PrintPartner 20W        600     600     BW      Ghostscript     -sDEVICE=ljet4 -r600

Fujitsu PrintPartner 20W        1200    1200    BW      Ghostscript     -sDEVICE=ljet4 -r1200

Fujitsu PrintPartner 8000       300     300     BW      Postscript              
HP      2000C   600     600     Color   Ghostscript     -sDEVICE=hpdj   
HP      2500C   600     600     Color   Ghostscript     -sDEVICE=cdj550 
HP      Color LaserJet 4500     600     600     Color   Postscript              4500N can use s
tandard postcript driver
HP      DesignJet 650C  300     300     Color   Ghostscript     -sDEVICE=dnj650c        
HP      DeskJet 1100C   300     300     Color   Ghostscript     -sDEVICE=cdj850 
HP      DeskJet 1120C   600     600     Color   Ghostscript     -sDEVICE=hpdj   
HP      DeskJet 1600C   600     600     Color   Ghostscript     -sDEVICE=cdj1600        
HP      DeskJet 400     300     300     BW      Ghostscript     -sDEVICE=cdj500 -dBitsPerPixel=
1       regular bw printing
HP      DeskJet 400     300     300     Color   Ghostscript     -sDEVICE=cdj500 -dBitsPerPixel=
3       regular color printing
HP      DeskJet 400     300     300     BW      Ghostscript     -sDEVICE=cdj500 -dBitsPerPixel=
8       dithered bw for better greys
HP      DeskJet 400     300     300     Color   Ghostscript     -sDEVICE=cdj500 -dBitsPerPixel=
24      dithered color
HP      DeskJet 500     300     300     BW      Ghostscript     -sDEVICE=djet500        regular
 printing
HP      DeskJet 500     300     300     BW      Ghostscript     -sDEVICE=djet500 -dBitsPerPixel
=8      dithered for better greyscale
HP      DeskJet 500C    300     300     BW      Ghostscript     -sDEVICE=cdj500 -dBitsPerPixel=
1       plain bw printing
HP      DeskJet 500C    300     300     Color   Ghostscript     -sDEVICE=cdj500 -dBitsPerPixel=
3       plain cmy color printing
HP      DeskJet 500C    300     300     BW      Ghostscript     -sDEVICE=cdj500 -dBitsPerPixel=
8       dithered bw printing; better greys
HP      DeskJet 500C    300     300     Color   Ghostscript     -sDEVICE=cdj500 -dBitsPerPixel=
24      dithered cmy printing
HP      DeskJet 520     300     300     BW      Ghostscript     -sDEVICE=hpdj   
HP      DeskJet 540     300     300     Color   Postscript      -sDEVICE=cdj500 
HP      DeskJet 550C    300     300     BW      Ghostscript     -sDEVICE=djet500 -dBitsPerPixel
=1      regular bw mode
HP      DeskJet 550C    300     300     Color   Ghostscript     -sDEVICE=djet500 -dBitsPerPixel
=3      regular color mode
HP      DeskJet 550C    300     300     BW      Ghostscript     -sDEVICE=djet500 -dBitsPerPixel
=8      dithered bw for better grey
HP      DeskJet 550C    300     300     Color   Ghostscript     -sDEVICE=djet500 -dBitsPerPixel
=24     dithered color mode
HP      DeskJet 550C    300     300     BW      Ghostscript     -sDEVICE=djet500 -dBitsPerPixel
=1 -dDepletion=2        regular bw mode using less ink
HP      DeskJet 550C    300     300     Color   Ghostscript     -sDEVICE=djet500 -dBitsPerPixel
=24 -dDepletion=2       dithered color mode using less ink
HP      DeskJet 560C    300     300     Color   Ghostscript     -sDEVICE=cdj550 
HP      DeskJet 600     600     600     BW      Ghostscript     -sDEVICE=cdj500 -r600x600 -dBit
sPerPixel=1 -dDepletion=2       
HP      DeskJet 600     600     600     Color   Ghostscript     -sDEVICE=cdj550 -r600x600 -dBit
sPerPixel=3 -dDepletion=2       
HP      DeskJet 600     600     600     BW      Ghostscript     -sDEVICE=cdj550 -r600x600 -dBit
sPerPixel=8 -dDepletion=2       dithered greyscale
HP      DeskJet 600     600     600     Color   Ghostscript     -sDEVICE=cdj550 -r600x600 -dBit
sPerPixel=24 -dDepletion=2      dithered full color
HP      DeskJet 610C    600     600     Color   Ghostscript     -sDEVICE=cdj670 for normal colo
r printing
HP      DeskJet 610C    600     600     BW      Ghostscript     -sDEVICE=hpdj -sColorMode=mono
for monochrome printing
HP      DeskJet 610C    600     600     BW      Ghostscript     -sDEVICE=hpdj -sColorMode=mono 
-sPrintQuality=-1       for monochrome draft printing
HP      DeskJet 610CL   600     600     Color   Ghostscript     -sDEVICE=cdj550 
HP      DeskJet 612C    300     300     BW      Ghostscript     -sDEVICE=djet500 -r300x300 -dDe
pletion=3 -dBitsPerPixel=1      econo mode eqivalent - still looks pretty good
HP      DeskJet 612C    600     600     Color   Ghostscript     -sDEVICE=cdj550 -r600x600 -dDep
letion=2        general command form - add bits per pixel spec (3=low color, 8=nice BW, 24 and 
32=nice color)
HP      DeskJet 660C    600     600     Color   Ghostscript     -sDEVICE=hpdj -sModel=unspec

HP      DeskJet 670C    600     600     Color   Ghostscript     -sDEVICE=cdj670 
HP      DeskJet 672C    300     300     Color   Ghostscript     -sDEVICE=cdj550 
HP      DeskJet 672C    300     300     Color   Ghostscript     -sDEVICE=cdj550 -dBitsPerPixel=
24      dithered full color
HP      DeskJet 672C    300     300     Color   Ghostscript     -sDEVICE=cdj670 -dQuality=1
presentation quality
HP      DeskJet 672C    300     300     Color   Ghostscript     -sDEVICE=cdj670 -dQuality=1 -dP
apertype=1      presentation quality on bond paper
HP      DeskJet 672C    300     300     Color   Ghostscript     -sDEVICE=cdj670 -dRetStatus=0
normal paper w/ C-Ret off
HP      DeskJet 672C    300     300     Color   Ghostscript     -sDEVICE=cdj670 -dMasterGamma=1
.7 -dQuality=1  darker, for transparencies
HP      DeskJet 672C    600     600     BW      Ghostscript     -sDEVICE=cdj550 -r600x600 -dBit
sPerPixel=1     
HP      DeskJet 672C    600     600     BW      Ghostscript     -sDEVICE=cdj550 -r600x600 -dBit
sPerPixel=8     dithered for better greyscale
HP      DeskJet 672C    600     600     Color   Ghostscript     -sDEVICE=cdj670 
HP      DeskJet 690C    300     300     Color   Postscript      -sDEVICE=cdj670 
HP      DeskJet 692C    300     300     Color   Ghostscript     -sDEVICE=cdj670 
HP      DeskJet 694C    300     300     BW      Ghostscript     -sDEVICE=hpdj -sPrintQualit=-1
High speed text and crappy B&W graphics
HP      DeskJet 694C    300     300     Color   Ghostscript     @cdj550.upp     "Normal" qualit
y color images.
HP      DeskJet 694C    600     600     Color   Ghostscript     @       
HP      DeskJet 697C    600     600     Color   Ghostscript     -sDEVICE=cdj550 
HP      DeskJet 710C    600     600     Color   Postscript      -sDEVICE=pbm2ppa        
HP      DeskJet 710C    600     600     Color   Postscript      -sDEVICE=pbm2ppa        
HP      DeskJet 720C    600     600     Color   Ghostscript     -sDEVICE=pbm2ppa        
HP      DeskJet 722C    600     300     BW      Ghostscript     -sDEVICE=pbm2ppa        
HP      DeskJet 810C    600     300     Color   Ghostscript     -sDEVICE=cdj880 
HP      DeskJet 810C    600     300     Color   Ghostscript     -sDEVICE=cdj880 
HP      DeskJet 810C    600     300     BW      Ghostscript     -sDEVICE=cdj880 
HP      DeskJet 812C    600     600     Color   Ghostscript     -sDEVICE=cdj880 
HP      DeskJet 812C    600     600     Color   DVI     -sDEVICE=cdj880 
HP      DeskJet 812C    600     600     Color   Ghostscript     -sDEVICE=cdj880 
HP      DeskJet 812C    600     600     Color   DVI     -sDEVICE=cdj880 
HP      DeskJet 812C    600     600     Color   Postscript      -sDEVICE=cdj880 
HP      DeskJet 812C    600     600     Color   DVI     -sDEVICE=cdj880 
HP      DeskJet 820C    600     600     Color   Ghostscript     -sDEVICE=pbm2ppa        
HP      DeskJet 832C    600     600     Color   Ghostscript     -sDEVICE=cdj880 
HP      DeskJet 832C    600     600     Color   Ghostscript     -sDEVICE=cdj550 
HP      DeskJet 850C    600     600     Color   Ghostscript     -sDEVICE=cdj850 Hp DeskJet 850C
HP      DeskJet 855C    600     600     Color   Ghostscript     -sDEVICE=cdj850 
HP      DeskJet 870C    600     600     Color   Ghostscript     -dQuality=0 -sDEVICE=cdj850 -dB
itsPerPixel=32  
HP      DeskJet 870C    600     600     Color   Ghostscript     -sDEVICE=cdj850 
HP      DeskJet 880C    600     600     Color   DVI     -sDEVICE=cdj880 -dGammaValC=3.0 -dGamma
ValY=3.0        
HP      DeskJet 880C    600     600     Color   Ghostscript     -sDEVICE=cdj880 
HP      DeskJet 882C    600     600     Color   Ghostscript     -sDEVICE=cdj880 
HP      DeskJet 890     600     600     Color   Postscript              
HP      DeskJet 890     600     600     Color   Ghostscript     -sDEVICE=cdj890 
HP      DeskJet 890C    600     600     Color   Ghostscript     -sDEVICE=cdj890 
HP      DeskJet 890C    600     600     Color   Ghostscript     -sDEVICE=hpdj -sModel=unspec
Appears to be a good deal faster than the cdj890 device
HP      DeskJet 895Cxi  600     600     Color   Ghostscript     -sDEVICE=cdj880 
HP      DeskJet 970C    600     600     Color   Ghostscript     -sDEVICE=cdj550 
HP      DeskJet 970Cse  600     600     Color   Ghostscript     -sDEVICE=cdj550 -r600x600

HP      LaserJet        300     300     BW      Ghostscript     -sDEVICE=laserjet       
HP      LaserJet 1100   600     600     BW      Ghostscript     -sDEVICE=ljet4  
HP      LaserJet 1100A  600     600     BW      Ghostscript     -sDEVICE=ljet4  
HP      LaserJet 2 w/PS 300     300     BW      Postscript              with postscript cartrid
ge
HP      LaserJet 2 w/PS 300     300     BW      Ghostscript     -sDEVICE=laserjet       native 
pcl
HP      LaserJet 2100   600     600     BW      Ghostscript     -sDEVICE=ljet4  
HP      LaserJet 2100M  1200    1200    BW      Postscript              
HP      LaserJet 2D     300     300     BW      Ghostscript     -sDEVICE=ljet2p 
HP      LaserJet 2P     150     150     BW      Ghostscript     -sDEVICE=ljet2p -r150x150
with <256k RAM this is necessary
HP      LaserJet 2P     300     300     BW      Ghostscript     -sDEVICE=ljet2p 
HP      LaserJet 3      300     300     BW      Ghostscript     -sDEVICE=ljet3  
HP      LaserJet 3      300     300     BW      Ghostscript     -sDEVICE=ljet3  
HP      LaserJet 3D     300     300     BW      Ghostscript     -sDEVICE=ljet3d 
HP      LaserJet 3P w/PS        300     300     BW      Postscript              w/postscript ca
rtridge
HP      LaserJet 4 Plus 600     600     BW      Postscript      -sDEVICE=ljet4  Ordered dither 
Grayscale
HP      LaserJet 4 Plus 600     600     BW      Ghostscript     -sDEVICE=lj4dith        Floyd-S
teinberg dither Grayscale
HP      LaserJet 4050N  1200    1200    BW      Postscript              
HP      LaserJet 4L     300     300     BW      Ghostscript     -sDEVICE=ljet4  
HP      LaserJet 4M     600     600     BW      Postscript              
HP      LaserJet 4ML    600     600     BW      Ghostscript     -sDEVICE=ljet4  
HP      LaserJet 4ML    600     600     BW      DVI     dvilj4 -q -w -  direct TeX dvi driver
HP      LaserJet 4ML    600     600     BW      Ghostscript     -sDEVICE=ljet4  
HP      LaserJet 5      600     600     BW      Ghostscript     -sDEVICE=ljet4  
HP      LaserJet 5000   1200    1200    BW      Postscript              
HP      LaserJet 5L     600     600     BW      Ghostscript     -sDEVICE=ljet4  
HP      LaserJet 5L     600     600     BW      Ghostscript     -sDEVICE=lj4dith        Better 
dithering, but needs more printer memory
HP      LaserJet 5M     600     600     BW      Postscript              
HP      LaserJet 5MP    600     600     BW      Postscript      command accepting Postscript

HP      LaserJet 5P     600     600     BW      Ghostscript     -sDEVICE=ljet4  
HP      LaserJet 6      600     600     BW      Ghostscript     -sDEVICE=ljet4  
HP      LaserJet 6      600     600     BW      Ghostscript     -sDEVICE=ljet4  
HP      LaserJet 6L     600     600     BW      DVI     -sDEVICE=ljet4  
HP      LaserJet 6L     600     600     BW      Ghostscript     -sDEVICE=ljet4  
HP      LaserJet 6MP    600     600     BW      Postscript              you may want more than 
1M RAM
HP      LaserJet 6P     600     600     BW      Postscript      -sDEVICE=ljet4  gray
HP      LaserJet 8000   1200    1200    BW      Postscript              
HP      LaserJet Plus   300     300     BW      Ghostscript     -sDEVICE=ljetplus       
HP      OfficeJet 600   300     300     Color   Ghostscript     -sDEVICE=cdj550 might do 600x60
0 in B&W mode ?!
HP      OfficeJet 625   600     600     Color   Postscript      -sDEVICE=hpdj   
HP      OfficeJet Pro 1150C     600     600     Color   Ghostscript             
HP      OfficeJet Pro 1170Cse   300     300     Color   Postscript      lpr     No need for ext
ra params
HP      PaintJet        300     300     Color   Ghostscript     -sDEVICE=pj     
HP      PaintJet XL     300     300     Color   Ghostscript     -sDEVICE=pjxl   
HP      PhotoSmart P1100        600     600     Color   Ghostscript     -sDEVICE=cdj550 
IBM     3853 JetPrinter 82      82      BW      Ghostscript     -sDEVICE=jetp3852       
IBM     4019    300     300     BW      Ghostscript     -sDEVICE=laserjet       IBM LaserPrinte
r 4019
IBM     Page Printer 3112       1200    1200    BW      Postscript              
IBM     Page Printer 3112       1200    1200    BW      DVI     dvips -D1200    
Kyocera FS-1700+        600     600     BW      Ghostscript             
Kyocera FS-3500 300     300     BW      Ghostscript     -sDEVICE=ljet3  
Kyocera FS-600  300     300     BW      Ghostscript     -sDEVICE=ljet4 -r300x300        maybe n
eeded for small memory?
Kyocera FS-600  600     600     BW      Ghostscript     -sDEVICE=ljet4 -r600x600        may req
uire over 4MB for some jobs
Lexmark 1000    300     300     Color   Ghostscript     -sDEVICE=ppmraw -sOutputFile=- -r300 | 
lm1100 -        
Lexmark 1020 Business   600     300     Color   Ghostscript     -sDEVICE=hpdj   
Lexmark 1100    300     300     Color   Ghostscript     -sDEVICE=ppmraw -sOutputFile=- | lm1100
 -      
Lexmark 2030    300     300     Color   Postscript      gs -sDEVICE=pbmraw -q -dNOPAUSE -r300 -
dSAFER -sOutputFile=- - | pbm2ppa       pbm2ppa is what pbm2l2030 installs as
Lexmark 2070    600     600     Color   Ghostscript     -sDEVICE=bitcmyk -dDITHERPPI=150
post filtered by c2070!
Lexmark 2070 Color Jetprinter   300     300     Color   Ghostscript     -sDEVICE=bitcmyk -dDITH
ERPPI=150       post filtered by c2070!
Lexmark 3000    300     300     Color   Ghostscript     -sDEVICE=cdj550 
Lexmark 3000 color jetprinter   1024    1024    Color   Ghostscript             
Lexmark 5000    300     600     Color   Ghostscript     -sDEVICE=lx5000 -r300x600       Require
s gdevlx50.c for GS
Lexmark 5000    300     600     BW      Ghostscript     -sDEVICE=lx5000 -r300x600 -dCMYK=false
Requires gdevlx50.c for GS
Lexmark 5000    600     600     BW      Postscript      gs -q -dSAFER -dNOPAUSE -sDEVICE=pbmraw
 -r600 -sOutputFile=- -dBATCH | pbm2l7k requires lexmark7000 package
Lexmark 5000    600     600     Color   Postscript      gs -dNOPAUSE -q -sDEVICE=pnmraw -sOutpu
tFile=- -dMaxBitmap=102000000 -r600 -dBATCH| pnmgamma 3.5 | pnmraw2cmyk -i -k | pbm2l7k require
s lexmark7000 package
Lexmark 5000    600     600     Color   Ghostscript     -sDEVICE=lx5000 requires gdevlx50.c for
 GS
Lexmark 5000    600     600     BW      Ghostscript     -sDEVICE=lx5000 -dCMYK=false    Require
s gdevlx50.c for GS
Lexmark 5700    600     600     BW      Postscript      gs -q -dSAFER -dNOPAUSE -sDEVICE=pbmraw
 -r600 -sOutputFile=- -dBATCH | pbm2l7k -m 1    with Henryk's l7k package (see Lexmark 7000)
Lexmark 5700    600     600     Color   Postscript      gs -dNOPAUSE -q -sDEVICE=pnmraw -sOutpu
tFile=- -dMaxBitmap=102000000-r600 -dBATCH | pnmgamma 3.5 | pnmraw2cmyk -i -k | pbm2l7k -m 1
broken in l7k 990418
Lexmark 5700    1200    1200    Color   Ghostscript     -sDEVICE=lxm5700m       
Lexmark 5700    1200    1200    BW      Ghostscript     -sDEVICE=lxm5700m -dHeadSeparation=15
set HeadSeparation per lxm5700m driver docs
Lexmark 7000    600     600     Color   Postscript      gs -dNOPAUSE -q -sDEVICE=pnmraw -sOutpu
tFile=- -dMaxBitmap=102000000 -r600 -dBATCH| pnmgamma 3.5 | pnmraw2cmyk -i -k | pbm2l7k require
s lexmark7000 package
Lexmark 7000    600     600     BW      Postscript      gs -q -dSAFER -dNOPAUSE -sDEVICE=pbmraw
 -r600 -sOutputFile=- -dBATCH | pbm2l7k requires lexmark7000 package
Lexmark 7200    600     600     Color   Postscript      gs -dNOPAUSE -q -sDEVICE=pnmraw -sOutpu
tFile=- -dMaxBitmap=102000000 -r600 -dBATCH| pnmgamma 3.5 | pnmraw2cmyk -i -k | pbm2l7k require
s lexmark7000 package
Lexmark 7200    600     600     BW      Postscript      gs -q -dSAFER -dNOPAUSE -sDEVICE=pbmraw
 -r600 -sOutputFile=- -dBATCH | pbm2l7k requires lexmark7000 package
Lexmark Optra Color 40  600     600     Color   Postscript              
Lexmark Optra Color 45  600     600     Color   Ghostscript             
Lexmark Optra E 600     600     BW      Postscript      -sDEVICE=ljet4  Stampante LexMark Optra
 E
Lexmark Optra E310      600     600     BW      Postscript      -sDEVICE=Lexmark2070    
Lexmark Optra Ep        600     600     BW      Postscript              
Lexmark Optra R+        1200    1200    BW      Postscript              
Lexmark Optra S 1250    1200    1200    BW      Postscript              
Lexmark Winwriter 400   300     300     BW      Ghostscript     -sDEVICE=ljetplus       
Minolta PagePro 6       600     600     BW      Postscript      -sDEVICE=ljet4  
Minolta PagePro 6e      600     600     BW      Ghostscript     -sDEVICE=ljet4  
Minolta PagePro 8L      600     600     BW      Ghostscript     -sDEVICE=ljet2p 
NEC     PinWriter P6    360     360     Color   Ghostscript     -sDEVICE=necp6  
NEC     PinWriter P6 plus       360     360     Color   Ghostscript     -sDEVICE=necp6  
NEC     PinWriter P60   360     360     Color   Ghostscript     -sDEVICE=necp6  
NEC     PinWriter P7    360     360     Color   Ghostscript     -sDEVICE=necp6  
NEC     PinWriter P7 plus       360     360     Color   Ghostscript     -sDEVICE=necp6  
NEC     PinWriter P70   360     360     Color   Ghostscript     -sDEVICE=necp6  
NEC     SilentWriter LC 890     300     300     BW      Postscript              
NEC     SuperScript 1260        600     600     Color   Ghostscript     -sDEVICE=ljet2p 
NEC     SuperScript 660i        600     600     BW      Ghostscript     -sDEVICE=ljet4  
NEC     SuperScript 870 600     600     BW      Postscript      -sDEVICE=ljet2p 
Oce     3165    600     600     BW      Postscript              
Okidata Microline 182   144     144     BW      Ghostscript     -sDEVICE=oki182 
Okidata OL 410e 600     600     BW      Ghostscript     -sDEVICE=ljet4  
Okidata OL 810e/PS      600     600     BW      Ghostscript             
Okidata OL810ex 600     600     BW      Postscript      -sDEVICE=ljet4  
Okidata OL830Plus       300     300     BW      Postscript              
Okidata OkiPage 4w+     300     300     BW      Ghostscript     -sDEVICE=oki4w  
Okidata Okipage 12i     600     600     BW      Postscript              
Okidata Okipage 20DXn   600     600     BW      Postscript              
Okidata Okipage 6e      600     600     BW      Ghostscript     -sDEVICE=ljet4  
Okidata Okipage 6ex     600     600     BW      Ghostscript     -sDEVICE=ljet4  
Okidata Super 6e        300     300     BW      Ghostscript     -sDEVICE=ljet4 -r300x300

Olivetti        JP350S  300     300     BW      Ghostscript     -sDEVICE=laserjet       
Olivetti        JP450   300     300     Color   Postscript      -sDEVICE=djet500        
Olivetti        PG 306  300     300     BW      Ghostscript     -sDEVICE=laserjet       
Panasonic       KX-P1123        360     360     BW      Ghostscript     -sDEVICE=epson  
Panasonic       KX-P2023        360     360     BW      Ghostscript     -sDEVICE=epson  prints 
using two passes per line
Panasonic       KX-P2123        360     360     Color   Ghostscript     -sDEVICE=epson  normal 
mode
Panasonic       KX-P4410        300     300     BW      Ghostscript     -sDEVICE=ljet2p 
Panasonic       KX-P4450        300     300     BW      Ghostscript     -sDEVICE=ljet3  
Panasonic       KX-P5400        300     300     BW      Postscript              
Panasonic       KX-P6150        300     300     BW      Ghostscript     -sDEVICE=laserjet
cuts off bottom?
Panasonic       KX-P6500        300     300     BW      Ghostscript     -sDEVICE=ljet2p regular
 printing mode
QMS     ps-810  300     300     BW      Postscript              
Ricoh   4081    300     300     BW      Ghostscript     -sDEVICE=r4081  
Sharp   AR-161  600     600     BW      Ghostscript     -sDEVICE=ljet4  
Star    LC24-100        1       1       BW      Ghostscript     please delete this line please 
delete this line
Star    LC24-100        360     360     BW      Ghostscript     -r360x360 -sDEVICE=necp6

Star    LC24-100        360     180     BW      Ghostscript     -r360x180 -sDEVICE=epson

Star    NL-10   60      72      BW      Ghostscript     -sDEVICE=epson -r60x72 -c '<<' /Margins
 [ -62 -45 ] /.HWMargins [ 18 0 0 67 ] '>>' setpagedevice       tractor feed
Star    NL-10   60      72      BW      Ghostscript     -sDEVICE=epson -r60x72 -c '<<' /Margins
 [ -60 -75 ] /.HWMargins [ 18 79 0 72 ] '>>' setpagedevice      single sheet feed
Star    NL-10   120     72      BW      Ghostscript     -sDEVICE=epson -r120x72 -c '<<' /Margin
s [ -62 -45 ] /.HWMargins [ 18 0 0 67 ] '>>' setpagedevice      tractor feed
Star    NL-10   120     72      BW      Ghostscript     -sDEVICE=epson -r120x72 -c '<<' /Margin
s [ -60 -75 ] /.HWMargins [ 18 79 0 72 ] '>>' setpagedevice     single sheet feed
Star    NL-10   240     216     BW      Ghostscript     -sDEVICE=eps9high -r240x216 -c '<<' /Ma
rgins [ -62 -135 ] /.HWMargins [ 18 0 0 67 ] '>>' setpagedevice tractor feed
Star    NL-10   240     216     BW      Ghostscript     -sDEVICE=eps9high -r240x216 -c '<<' /Ma
rgins [ -60 -225 ] /.HWMargins [ 18 79 0 72 ] '>>' setpagedevice        single sheet feed
Star    StarJet 48      360     360     BW      Ghostscript     -sDEVICE=sj48   
Tektronix       3693d color printer, 8-bit mode 100     100     Color   Ghostscript     -sDEVIC
E=t4693d8       
Tektronix       4693d color printer, 2-bit mode 100     100     Color   Ghostscript     -sDEVIC
E=t4693d2       
Tektronix       4695    120     120     BW      Ghostscript     -sDEVICE=tek4696        
Tektronix       4696    180     180     Color   Ghostscript     -sDEVICE=tek4696        
Tektronix       4697    216     216     Color   Ghostscript     -sDEVICE=tek4696        
Tektronix       Phaser IISX     300     300     Color   Postscript              
Tektronix       Phaser PX       300     300     Color   Postscript              
Xerox   2700 XES        300     300     BW      Ghostscript     -sDEVICE=xes    
Xerox   3700 XES        300     300     BW      Ghostscript     -sDEVICE=xes    
Xerox   4045 XES        300     300     BW      Ghostscript     -sDEVICE=xes    
Xerox   DocuPrint 4508  600     600     BW      Postscript      -sDEVICE=ljet4  
Xerox   DocuPrint 4508  600     600     BW      Ghostscript     -sDEVICE=ljet4  
Xerox   DocuPrint 4508  600     600     BW      DVI     -sDEVICE=ljet4  
Xerox   DocuPrint P12   300     300     BW      Ghostscript     -sDEVICE=ljet2p 
Xerox   DocuPrint P8e   600     600     BW      Postscript      -sDEVICE=ljet4  
Xerox   Document Homecentre     600     600     Color   Ghostscript     -sDEVICE=cdj550 
Xerox   WorkCentre 450cp        300     300     Color   Ghostscript     -sDEVICE=cdj550 

