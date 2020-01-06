************************************************************************
* auCdl Netlist:
* 
* Library Name:  OTA_class
* Top Cell Name: single_ended_current_mirror_pmos
* View Name:     schematic
* Netlisted on:  Sep 11 21:07:49 2019
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
* Cell Name:    single_ended_current_mirror_pmos
* View Name:    schematic
************************************************************************

.SUBCKT single_ended_current_mirror_pmos Vbiasp Vinn Vinp Voutp
*.PININFO Vbiasp:I Vinn:I Vinp:I Voutp:O
MM3 Voutp net17 vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
MM2 net17 net17 vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
MM7 net9 Vinn net13 net22 pmos_rvt w=WA l=LA nfin=nA
MM6 net15 Vinp net13 net22 pmos_rvt w=WA l=LA nfin=nA
MM5 net13 Vbiasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
MM1 Voutp net9 gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM0 net17 net15 gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM9 net9 net9 gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM8 net15 net15 gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
.ENDS


.SUBCKT LG_pmos Biasn Vbiasp
*.PININFO Biasn:I Vbiasp:O
MM10 Vbiasp Biasn gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM3 Vbiasp Vbiasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
.ENDS

.SUBCKT CR2_2_wilson Vbiasn Vbiasp
*.PININFO Vbiasn:O Vbiasp:O
RR0 Vbiasn gnd! res=rK
MM2 Vbiasp net12 Vbiasn gnd! nmos_rvt w=WA l=LA nfin=nA
MM0 net12 Vbiasn gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM3 Vbiasp Vbiasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
MM1 net12 Vbiasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
.ENDS


xiota LG_Vbiasp Vinn Vinp Voutp single_ended_current_mirror_pmos
xiLG_pmos Biasn LG_Vbiasp LG_pmos
xibCR2_2_wilson Biasn Vbiasp CR2_2_wilson
.END