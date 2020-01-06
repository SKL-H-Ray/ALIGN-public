************************************************************************
* auCdl Netlist:
* 
* Library Name:  OTA_class
* Top Cell Name: single_ended_low_voltage_cascode_current_mirror
* View Name:     schematic
* Netlisted on:  Sep 11 21:08:07 2019
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
* Cell Name:    single_ended_low_voltage_cascode_current_mirror
* View Name:    schematic
************************************************************************

.SUBCKT single_ended_low_voltage_cascode_current_mirror Vbiasn Vinn Vinp Voutp
*.PININFO Vbiasn:I Vinn:I Vinp:I Voutp:O
MM3 Voutp Vinp net13 gnd! nmos_rvt w=WA l=LA nfin=nA
MM0 net11 Vinn net13 gnd! nmos_rvt w=WA l=LA nfin=nA
MM4 net13 Vbiasn gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM6 net11 net18 net19 vdd! pmos_rvt w=WA l=LA nfin=nA
MM5 Voutp net18 net20 vdd! pmos_rvt w=WA l=LA nfin=nA
MM1 net20 net11 vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
MM2 net19 net11 vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
.ENDS


.SUBCKT LG_nmos Biasp Vbiasn
*.PININFO Biasp:I Vbiasn:O
MM8 Vbiasn Vbiasn gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM10 Vbiasn Biasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
.ENDS

.SUBCKT CR3_2 Vbiasn Vbiasp
*.PININFO Vbiasn:O Vbiasp:O
MM3 Vbiasp Vbiasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
MM1 Vbiasn Vbiasp vdd! vdd! pmos_rvt w=WA l=LA nfin=nA
RR0 net15 gnd! res=rK
MM0 Vbiasn Vbiasn gnd! gnd! nmos_rvt w=WA l=LA nfin=nA
MM2 Vbiasp Vbiasn net15 gnd! nmos_rvt w=WA l=LA nfin=nA
.ENDS


xiota LG_Vbiasn Vinn Vinp Voutp single_ended_low_voltage_cascode_current_mirror
xiLG_nmos Biasp LG_Vbiasn LG_nmos
xibCR3_2 Biasn Biasp CR3_2
.END