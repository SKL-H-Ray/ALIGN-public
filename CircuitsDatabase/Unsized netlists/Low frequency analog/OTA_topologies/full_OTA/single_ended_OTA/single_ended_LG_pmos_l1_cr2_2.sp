************************************************************************
* auCdl Netlist:
* 
* Library Name:  OTA_class
* Top Cell Name: single_ended
* View Name:     schematic
* Netlisted on:  Sep 11 21:06:14 2019
************************************************************************

*.BIPOLAR
*.RESI = 2000 
*.RESVAL
*.CAPVAL
*.DIOPERI
*.DIOAREA
*.EQUATION
*.SCALE METER
*.MEGA
.PARAM

*.GLOBAL vdd!
+        gnd!

*.PIN vdd!
*+    gnd!

************************************************************************
* Library Name: OTA_class
* Cell Name:    single_ended
* View Name:    schematic
************************************************************************

.SUBCKT single_ended Vbiasn Vinn Vinp Voutp
*.PININFO Vbiasn:I Vinn:I Vinp:I Voutp:O
MM3 Voutp Vinp net16 gnd! nmos_rvt w=WA l=LA nfin=nA
MM0 Vbiasp Vinn net16 gnd! nmos_rvt w=WA l=LA nfin=nA
MM4 net16 Vbiasn gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM1 Voutp Vbiasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
MM2 Vbiasp Vbiasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
.ENDS


.SUBCKT LG_pmos_l1 Biasp Vbiasn Vbiasp
*.PININFO Biasp:I Vbiasn:O Vbiasp:O
MM0 Vbiasp Vbiasn gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM8 Vbiasn Vbiasn gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM3 Vbiasp Vbiasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
MM10 Vbiasn Biasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
.ENDS

.SUBCKT CR2_2_wilson Vbiasn Vbiasp
*.PININFO Vbiasn:O Vbiasp:O
RR0 Vbiasn gnd! res=rK
MM2 Vbiasp net12 Vbiasn gnd! nmos_rvt w=WA l=LA nfin=nA
MM0 net12 Vbiasn gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM3 Vbiasp Vbiasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
MM1 net12 Vbiasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
.ENDS


xiota LG_Vbiasn Vinn Vinp Voutp single_ended
xiLG_pmos_l1 Biasp LG_Vbiasn LG_Vbiasp LG_pmos_l1
xibCR2_2_wilson Biasn Biasp CR2_2_wilson
.END