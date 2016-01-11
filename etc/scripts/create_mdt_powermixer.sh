#!/bin/bash
#  changed to use bash shell ; as there is echo -e option issue in sh shell
#  only linux will use bash shell ; aix will continue to use sh.
#  this script should be common to all linux releases. 

# Code to create mdt.pm_cmp that points to powerMixer.cmp.config.txt
os=`uname`
if test $os = "Linux"
then
#  we  need -e option to use escape sequence in gnu echo statement 
 opt="-e";
fi

echo $opt  'pwrmxr:'                                                                  > /tmp/pwrmxr_stanza
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'  >> /tmp/pwrmxr_stanza
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.' >> /tmp/pwrmxr_stanza
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'  >> /tmp/pwrmxr_stanza
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'        >> /tmp/pwrmxr_stanza
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.cmp.config.txt" * reg'           >> /tmp/pwrmxr_stanza


cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_cmp
cat /tmp/pwrmxr_stanza                                                         >> ${HTXMDT}mdt.pm_cmp
echo $opt                                                                            >> ${HTXMDT}mdt.pm_cmp

ln -sf /usr/lpp/htx/mdt/mdt.pm_cmp /usr/lpp/htx/ecg/ecg.pm_cmp


#cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_msrp

#echo $opt  'pwrmxr:'                                                                 >> ${HTXMDT}mdt.pm_msrp
#echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'  >> ${HTXMDT}mdt.pm_msrp
#echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.' >> ${HTXMDT}mdt.pm_msrp
#echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'  >> ${HTXMDT}mdt.pm_msrp
#echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'        >> ${HTXMDT}mdt.pm_msrp
#echo $opt  '\treg_rules = "hxepowermixer/powerMixer.msrp.config.txt" * reg'          >> ${HTXMDT}mdt.pm_msrp
#echo $opt                                                                            >> ${HTXMDT}mdt.pm_msrp

#ln -sf /usr/lpp/htx/mdt/mdt.pm_msrp /usr/lpp/htx/ecg/ecg.pm_msrp


# Code to create mdt.pm_msrp_atlas that points to powerMixer.msrp.atlas.config.txt

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_msrp_atlas

echo $opt  'pwrmxr:'                                                                 >> ${HTXMDT}mdt.pm_msrp_atlas
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'  >> ${HTXMDT}mdt.pm_msrp_atlas
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.' >> ${HTXMDT}mdt.pm_msrp_atlas
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'  >> ${HTXMDT}mdt.pm_msrp_atlas
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'        >> ${HTXMDT}mdt.pm_msrp_atlas
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.msrp.atlas.config.txt" * reg'    >> ${HTXMDT}mdt.pm_msrp_atlas
echo $opt                                                                            >> ${HTXMDT}mdt.pm_msrp_atlas

ln -sf /usr/lpp/htx/mdt/mdt.pm_msrp_atlas /usr/lpp/htx/ecg/ecg.pm_msrp_atlas


# Code to create mdt.pm_msrp_saturn that points to powerMixer.msrp.saturn.config.txt

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                             > ${HTXMDT}mdt.pm_msrp_saturn

echo $opt  'pwrmxr:'                                                                 >> ${HTXMDT}mdt.pm_msrp_saturn
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'  >> ${HTXMDT}mdt.pm_msrp_saturn
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.' >> ${HTXMDT}mdt.pm_msrp_saturn
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'  >> ${HTXMDT}mdt.pm_msrp_saturn
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'        >> ${HTXMDT}mdt.pm_msrp_saturn
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.msrp.saturn.config.txt" * reg'   >> ${HTXMDT}mdt.pm_msrp_saturn
echo $opt                                                                            >> ${HTXMDT}mdt.pm_msrp_saturn

ln -sf /usr/lpp/htx/mdt/mdt.pm_msrp_saturn /usr/lpp/htx/ecg/ecg.pm_msrp_saturn


# Code to create mdt.pm_msrp_titan that points to powerMixer.msrp.titan.config.txt

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_msrp_titan

echo $opt  'pwrmxr:'                                                                 >> ${HTXMDT}mdt.pm_msrp_titan
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'  >> ${HTXMDT}mdt.pm_msrp_titan
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.' >> ${HTXMDT}mdt.pm_msrp_titan
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'  >> ${HTXMDT}mdt.pm_msrp_titan
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'        >> ${HTXMDT}mdt.pm_msrp_titan
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.msrp.titan.config.txt" * reg'    >> ${HTXMDT}mdt.pm_msrp_titan
echo $opt                                                                            >> ${HTXMDT}mdt.pm_msrp_titan

ln -sf /usr/lpp/htx/mdt/mdt.pm_msrp_titan /usr/lpp/htx/ecg/ecg.pm_msrp_titan


# Code to create mdt.pm_1pv that points to powerMixer.1pv.config.txt
# added on request from diyanesh

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_1pv

echo $opt  'pwrmxr:'                                                                 >> ${HTXMDT}mdt.pm_1pv
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'  >> ${HTXMDT}mdt.pm_1pv
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.' >> ${HTXMDT}mdt.pm_1pv
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'  >> ${HTXMDT}mdt.pm_1pv
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'        >> ${HTXMDT}mdt.pm_1pv
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.1pv.config.txt" * reg'           >> ${HTXMDT}mdt.pm_1pv
echo $opt                                                                            >> ${HTXMDT}mdt.pm_1pv

ln -sf /usr/lpp/htx/mdt/mdt.pm_1pv /usr/lpp/htx/ecg/ecg.pm_1pv


# Code to create mdt.pm_2pv that points to powerMixer.2pv.config.txt
# added on request from diyanesh

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_2pv

echo $opt  'pwrmxr:'                                                                 >> ${HTXMDT}mdt.pm_2pv
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'  >> ${HTXMDT}mdt.pm_2pv
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.' >> ${HTXMDT}mdt.pm_2pv
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'  >> ${HTXMDT}mdt.pm_2pv
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'        >> ${HTXMDT}mdt.pm_2pv
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.2pv.config.txt" * reg'           >> ${HTXMDT}mdt.pm_2pv
echo $opt                                                                            >> ${HTXMDT}mdt.pm_2pv

ln -sf /usr/lpp/htx/mdt/mdt.pm_2pv /usr/lpp/htx/ecg/ecg.pm_2pv



# Code to create mdt.pm_3pv that points to powerMixer.3pv.config.txt
# added on request from diyanesh

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_3pv

echo $opt  'pwrmxr:'                                                                 >> ${HTXMDT}mdt.pm_3pv
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'  >> ${HTXMDT}mdt.pm_3pv
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.' >> ${HTXMDT}mdt.pm_3pv
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'  >> ${HTXMDT}mdt.pm_3pv
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'        >> ${HTXMDT}mdt.pm_3pv
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.3pv.config.txt" * reg'           >> ${HTXMDT}mdt.pm_3pv
echo $opt                                                                            >> ${HTXMDT}mdt.pm_3pv

ln -sf /usr/lpp/htx/mdt/mdt.pm_3pv /usr/lpp/htx/ecg/ecg.pm_3pv


# Code to create mdt.pm_ips_28s_active_52s_idle that points to powerMixer.msrp.1p3m.ips.28s_active_52s_idle.config.txt
# added on request from  Nicole S Schwartz

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_ips_28s_active_52s_idle

echo $opt  'pwrmxr:'		                                                                		    >> ${HTXMDT}mdt.pm_ips_28s_active_52s_idle
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'  			  	    >> ${HTXMDT}mdt.pm_ips_28s_active_52s_idle
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'				    >> ${HTXMDT}mdt.pm_ips_28s_active_52s_idle
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'  				    >> ${HTXMDT}mdt.pm_ips_28s_active_52s_idle
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'      				    >> ${HTXMDT}mdt.pm_ips_28s_active_52s_idle
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.msrp.1p3m.ips.28s_active_52s_idle.config.txt" * reg'            >> ${HTXMDT}mdt.pm_ips_28s_active_52s_idle
echo $opt                                                                                                           >> ${HTXMDT}mdt.pm_ips_28s_active_52s_idle

ln -sf /usr/lpp/htx/mdt/mdt.pm_ips_28s_active_52s_idle /usr/lpp/htx/ecg/ecg.pm_ips_28s_active_52s_idle


# Code to create mdt.pm_msrp_firebird that points to powerMixer.msrp.firebird.config.txt

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_msrp_firebird

echo $opt  'pwrmxr:'                                                                 >> ${HTXMDT}mdt.pm_msrp_firebird
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'  >> ${HTXMDT}mdt.pm_msrp_firebird
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.' >> ${HTXMDT}mdt.pm_msrp_firebird
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'  >> ${HTXMDT}mdt.pm_msrp_firebird
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'        >> ${HTXMDT}mdt.pm_msrp_firebird
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.msrp.firebird.config.txt" * reg' >> ${HTXMDT}mdt.pm_msrp_firebird
echo $opt                                                                            >> ${HTXMDT}mdt.pm_msrp_firebird

ln -sf /usr/lpp/htx/mdt/mdt.pm_msrp_firebird /usr/lpp/htx/ecg/ecg.pm_msrp_firebird

############################################################################################
# MDTs specific to P8!
############################################################################################


# Code to create mdt.pm_p8_rdp_smt1 that points to powerMixer.p8.rdp.smt1.config.txt
cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_p8_rdp_smt1

echo $opt  'pwrmxr:'                                                                 >> ${HTXMDT}mdt.pm_p8_rdp_smt1
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'  >> ${HTXMDT}mdt.pm_p8_rdp_smt1
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.' >> ${HTXMDT}mdt.pm_p8_rdp_smt1
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'  >> ${HTXMDT}mdt.pm_p8_rdp_smt1
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'        >> ${HTXMDT}mdt.pm_p8_rdp_smt1
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.p8.rdp.smt1.config.txt" * reg'   >> ${HTXMDT}mdt.pm_p8_rdp_smt1
echo $opt                                                                            >> ${HTXMDT}mdt.pm_p8_rdp_smt1
ln -sf /usr/lpp/htx/mdt/mdt.pm_p8_rdp_smt1 /usr/lpp/htx/ecg/ecg.pm_p8_rdp_smt1

#Code to create mdt.pm_p8_rdp_smt8 that points to powerMixer.p8.rdp.smt8.config.txt
cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_p8_rdp_smt8

echo $opt  'pwrmxr:'                                                                 >> ${HTXMDT}mdt.pm_p8_rdp_smt8
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'  >> ${HTXMDT}mdt.pm_p8_rdp_smt8
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.' >> ${HTXMDT}mdt.pm_p8_rdp_smt8
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'  >> ${HTXMDT}mdt.pm_p8_rdp_smt8
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'        >> ${HTXMDT}mdt.pm_p8_rdp_smt8
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.p8.rdp.smt8.config.txt" * reg'   >> ${HTXMDT}mdt.pm_p8_rdp_smt8
echo $opt                                                                            >> ${HTXMDT}mdt.pm_p8_rdp_smt8

ln -sf /usr/lpp/htx/mdt/mdt.pm_p8_rdp_smt8 /usr/lpp/htx/ecg/ecg.pm_p8_rdp_smt8

#Code to create mdt.pm_p8_rdp_smt4_swtch_400ms that points to powerMixer.p8.rdp.switching.config.txt
cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_p8_rdp_smt4_swtch_400ms

echo $opt  'pwrmxr:'                                                                 	>> ${HTXMDT}mdt.pm_p8_rdp_smt4_swtch_400ms
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'  	>> ${HTXMDT}mdt.pm_p8_rdp_smt4_swtch_400ms
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.' 	>> ${HTXMDT}mdt.pm_p8_rdp_smt4_swtch_400ms
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'  	>> ${HTXMDT}mdt.pm_p8_rdp_smt4_swtch_400ms
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'  		>> ${HTXMDT}mdt.pm_p8_rdp_smt4_swtch_400ms
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.p8.rdp.switching.config.txt" * reg' >> ${HTXMDT}mdt.pm_p8_rdp_smt4_swtch_400ms
echo $opt                                                                            	>> ${HTXMDT}mdt.pm_p8_rdp_smt4_swtch_400ms
ln -sf /usr/lpp/htx/mdt/mdt.pm_p8_rdp_smt4_swtch_400ms /usr/lpp/htx/ecg/ecg.pm_p8_rdp_smt4_swtch_400ms

#create mdt.pm_p8_rdp_smt4 , This uses P8 RDP 
# with less priority for powermixer

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_p8_rdp_smt4

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_rdp_smt4
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_p8_rdp_smt4
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_p8_rdp_smt4
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_p8_rdp_smt4
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_p8_rdp_smt4
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_p8_rdp_smt4
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.p8.rdp.smt4.config.txt" * reg'                      >> ${HTXMDT}mdt.pm_p8_rdp_smt4
echo $opt  '\tpriority = 17                     * priority (1=highest to 19=lowest)'                    >> ${HTXMDT}mdt.pm_p8_rdp_smt4
echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_rdp_smt4

ln -sf /usr/lpp/htx/mdt/mdt.pm_p8_rdp_smt4 /usr/lpp/htx/ecg/ecg.pm_p8_rdp_smt4


#create mdt.pm_p8_tdp_smt4 , This uses P8 TDP 
# with less priority for powermixer

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_p8_tdp_smt4

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_tdp_smt4
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_p8_tdp_smt4
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_p8_tdp_smt4
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_p8_tdp_smt4
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_p8_tdp_smt4
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_p8_tdp_smt4
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.p8.tdp.smt4.config.txt" * reg'                      >> ${HTXMDT}mdt.pm_p8_tdp_smt4
echo $opt  '\tpriority = 17                     * priority (1=highest to 19=lowest)'                    >> ${HTXMDT}mdt.pm_p8_tdp_smt4
echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_tdp_smt4

ln -sf /usr/lpp/htx/mdt/mdt.pm_p8_tdp_smt4 /usr/lpp/htx/ecg/ecg.pm_p8_tdp_smt4

#create mdt.pm_p8_tdp_smt1 , This uses P8 TDP 
# with less priority for powermixer

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_p8_tdp_smt1

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_tdp_smt1
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_p8_tdp_smt1
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_p8_tdp_smt1
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_p8_tdp_smt1
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_p8_tdp_smt1
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_p8_tdp_smt1
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.p8.tdp.smt1.config.txt" * reg'                      >> ${HTXMDT}mdt.pm_p8_tdp_smt1
echo $opt  '\tpriority = 17                     * priority (1=highest to 19=lowest)'                    >> ${HTXMDT}mdt.pm_p8_tdp_smt1
echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_tdp_smt1

ln -sf /usr/lpp/htx/mdt/mdt.pm_p8_tdp_smt1 /usr/lpp/htx/ecg/ecg.pm_p8_tdp_smt1


# code to create mdt.pm_p8_tdp_smt8 , This uses P8 TDP 

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_p8_tdp_smt8

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_tdp_smt8
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_p8_tdp_smt8
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_p8_tdp_smt8
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_p8_tdp_smt8
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_p8_tdp_smt8
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_p8_tdp_smt8
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.p8.tdp.smt8.config.txt" * reg'                      >> ${HTXMDT}mdt.pm_p8_tdp_smt8
echo $opt  '\tpriority = 17                     * priority (1=highest to 19=lowest)'                    >> ${HTXMDT}mdt.pm_p8_tdp_smt8
echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_tdp_smt8

ln -sf /usr/lpp/htx/mdt/mdt.pm_p8_tdp_smt8 /usr/lpp/htx/ecg/ecg.pm_p8_tdp_smt8
# code to create mdt.pm_p8_5fd_3md , This uses P8 fortran daxpy & memex_daxpy

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_p8_5fd_3md

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_5fd_3md
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_p8_5fd_3md
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_p8_5fd_3md
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_p8_5fd_3md
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_p8_5fd_3md
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_p8_5fd_3md
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.p8.5fd.3md.config.txt" * reg'                       >> ${HTXMDT}mdt.pm_p8_5fd_3md
echo $opt  '\tpriority = 17                     * priority (1=highest to 19=lowest)'                    >> ${HTXMDT}mdt.pm_p8_5fd_3md
echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_5fd_3md

if [[ $CMVC_RELEASE == "htxubuntu"  ||  $CMVC_RELEASE == "htxrhel72le" ]]; then
	echo $opt                                                                                           >> ${HTXMDT}mdt.pm_p8_5fd_3md
	create_my_mdt nvidia:rules.DGEMM | create_mdt_without_devices.awk default                           >> ${HTXMDT}mdt.pm_p8_5fd_3md
	echo $opt                                                                                           >> ${HTXMDT}mdt.pm_p8_5fd_3md
fi

ln -sf /usr/lpp/htx/mdt/mdt.pm_p8_5fd_3md /usr/lpp/htx/ecg/ecg.pm_p8_5fd_3md


#code to create a new mdt of mdt.pm_p8_7fd_1md
cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_p8_7fd_1md
echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_7fd_1md
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_p8_7fd_1md
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_p8_7fd_1md
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_p8_7fd_1md
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_p8_7fd_1md
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_p8_7fd_1md
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.p8.7fd.1md.config.txt" * reg'                       >> ${HTXMDT}mdt.pm_p8_7fd_1md
echo $opt  '\tpriority = 17                     * priority (1=highest to 19=lowest)'                    >> ${HTXMDT}mdt.pm_p8_7fd_1md
echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_7fd_1md

if [[ $CMVC_RELEASE == "htxubuntu"  ||  $CMVC_RELEASE == "htxrhel72le" ]]; then
	echo $opt                                                                                          >> ${HTXMDT}mdt.pm_p8_7fd_1md
	create_my_mdt nvidia:rules.DGEMM | create_mdt_without_devices.awk default                          >> ${HTXMDT}mdt.pm_p8_7fd_1md
	echo $opt                                                                                          >> ${HTXMDT}mdt.pm_p8_7fd_1md
fi


ln -sf /usr/lpp/htx/mdt/mdt.pm_p8_7fd_1md /usr/lpp/htx/ecg/ecg.pm_p8_7fd_1md


# code to create mdt.pm_p8_6fd_2md , This uses P8 fortran daxpy and memex daxpy

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_p8_6fd_2md

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_6fd_2md
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_p8_6fd_2md
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_p8_6fd_2md
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_p8_6fd_2md
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_p8_6fd_2md
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_p8_6fd_2md
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.p8.6fd.2md.config.txt" * reg'                       >> ${HTXMDT}mdt.pm_p8_6fd_2md
echo $opt  '\tpriority = 17                     * priority (1=highest to 19=lowest)'                    >> ${HTXMDT}mdt.pm_p8_6fd_2md
echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_6fd_2md

ln -sf /usr/lpp/htx/mdt/mdt.pm_p8_6fd_2md /usr/lpp/htx/ecg/ecg.pm_p8_6fd_2md

# code to create mdt.pm_p8_cmp , This uses P8 fortran daxpy

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_p8_cmp

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_cmp
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_p8_cmp
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_p8_cmp
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_p8_cmp
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_p8_cmp
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_p8_cmp
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.p8.8fd.config.txt" * reg'                           >> ${HTXMDT}mdt.pm_p8_cmp
echo $opt  '\tpriority = 10                     * priority (1=highest to 19=lowest)'                    >> ${HTXMDT}mdt.pm_p8_cmp
echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_cmp

ln -sf /usr/lpp/htx/mdt/mdt.pm_p8_cmp /usr/lpp/htx/ecg/ecg.pm_p8_cmp

# code to create mdt.pm_8fd_char, This uses  fortran daxpy 

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_8fd_char

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_8fd_char
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_8fd_char
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_8fd_char
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_8fd_char
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_8fd_char
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_8fd_char
echo $opt  '\treg_rules = "hxepowermixer/powerMixer.p8.8fd_char.config.txt" * reg'                      >> ${HTXMDT}mdt.pm_8fd_char

ln -sf /usr/lpp/htx/mdt/mdt.pm_8fd_char /usr/lpp/htx/ecg/ecg.pm_8fd_char

# MDT's added for the naples.

# MDT for the memex daxpy covering 64G
cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_naples_daxpy_64G

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_naples_daxpy_64G
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_naples_daxpy_64G
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_naples_daxpy_64G
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_naples_daxpy_64G
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_naples_daxpy_64G
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_naples_daxpy_64G
echo $opt  '\treg_rules = "hxepowermixer/powerMixer_p8_64G_memex_daxpy.txt" * reg'                      >> ${HTXMDT}mdt.pm_naples_daxpy_64G

ln -sf /usr/lpp/htx/mdt/mdt.pm_naples_daxpy_64G /usr/lpp/htx/ecg/ecg.pm_naples_daxpy_64G

# MDT for the memex daxpy covering 128G

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_naples_daxpy_128G

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_naples_daxpy_128G
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_naples_daxpy_128G
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_naples_daxpy_128G
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_naples_daxpy_128G
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_naples_daxpy_128G
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_naples_daxpy_128G
echo $opt  '\treg_rules = "hxepowermixer/powerMixer_p8_128G_memex_daxpy.txt" * reg'                     >> ${HTXMDT}mdt.pm_naples_daxpy_128G

# MDT for the memex daxpy covering 256G

ln -sf /usr/lpp/htx/mdt/mdt.pm_naples_daxpy_128G /usr/lpp/htx/ecg/ecg.pm_naples_daxpy_128G

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_naples_daxpy_256G

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_naples_daxpy_256G
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_naples_daxpy_256G
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_naples_daxpy_256G
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_naples_daxpy_256G
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_naples_daxpy_256G
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_naples_daxpy_256G
echo $opt  '\treg_rules = "hxepowermixer/powerMixer_p8_256G_memex_daxpy.txt" * reg'                      >> ${HTXMDT}mdt.pm_naples_daxpy_256G

ln -sf /usr/lpp/htx/mdt/mdt.pm_naples_daxpy_256G /usr/lpp/htx/ecg/ecg.pm_naples_daxpy_256G

# MDT for the memex daxpy covering 512G

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_naples_daxpy_512G

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_naples_daxpy_512G
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_naples_daxpy_512G
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_naples_daxpy_512G
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_naples_daxpy_512G
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_naples_daxpy_512G
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_naples_daxpy_512G
echo $opt  '\treg_rules = "hxepowermixer/powerMixer_p8_512G_memex_daxpy.txt" * reg'                      >> ${HTXMDT}mdt.pm_naples_daxpy_512G

ln -sf /usr/lpp/htx/mdt/mdt.pm_naples_daxpy_512G /usr/lpp/htx/ecg/ecg.pm_naples_daxpy_512G

# MDT for the memex daxpy covering 1024G

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_naples_daxpy_1024G

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_naples_daxpy_1024G
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_naples_daxpy_1024G
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_naples_daxpy_1024G
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_naples_daxpy_1024G
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_naples_daxpy_1024G
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_naples_daxpy_1024G
echo $opt  '\treg_rules = "hxepowermixer/powerMixer_p8_1024G_memex_daxpy.txt" * reg'                      >> ${HTXMDT}mdt.pm_naples_daxpy_1024G

ln -sf /usr/lpp/htx/mdt/mdt.pm_naples_daxpy_1024G /usr/lpp/htx/ecg/ecg.pm_naples_daxpy_1024G

# MDT for the memex ddot covering 64G
cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_naples_ddot_64G

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_naples_ddot_64G
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_naples_ddot_64G
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_naples_ddot_64G
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_naples_ddot_64G
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_naples_ddot_64G
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_naples_ddot_64G
echo $opt  '\treg_rules = "hxepowermixer/powerMixer_p8_64G_memex_ddot.txt" * reg'                      >> ${HTXMDT}mdt.pm_naples_ddot_64G

ln -sf /usr/lpp/htx/mdt/mdt.pm_naples_ddot_64G /usr/lpp/htx/ecg/ecg.pm_naples_ddot_64G

# MDT for the memex ddot covering 128G

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_naples_ddot_128G

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_naples_ddot_128G
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_naples_ddot_128G
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_naples_ddot_128G
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_naples_ddot_128G
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_naples_ddot_128G
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_naples_ddot_128G
echo $opt  '\treg_rules = "hxepowermixer/powerMixer_p8_128G_memex_ddot.txt" * reg'                     >> ${HTXMDT}mdt.pm_naples_ddot_128G

# MDT for the memex ddot covering 256G

ln -sf /usr/lpp/htx/mdt/mdt.pm_naples_ddot_128G /usr/lpp/htx/ecg/ecg.pm_naples_ddot_128G

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_naples_ddot_256G

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_naples_ddot_256G
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_naples_ddot_256G
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_naples_ddot_256G
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_naples_ddot_256G
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_naples_ddot_256G
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_naples_ddot_256G
echo $opt  '\treg_rules = "hxepowermixer/powerMixer_p8_256G_memex_ddot.txt" * reg'                      >> ${HTXMDT}mdt.pm_naples_ddot_256G

ln -sf /usr/lpp/htx/mdt/mdt.pm_naples_ddot_256G /usr/lpp/htx/ecg/ecg.pm_naples_ddot_256G

# MDT for the memex ddot covering 512G

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_naples_ddot_512G

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_naples_ddot_512G
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_naples_ddot_512G
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_naples_ddot_512G
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_naples_ddot_512G
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_naples_ddot_512G
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_naples_ddot_512G
echo $opt  '\treg_rules = "hxepowermixer/powerMixer_p8_512G_memex_ddot.txt" * reg'                      >> ${HTXMDT}mdt.pm_naples_ddot_512G

ln -sf /usr/lpp/htx/mdt/mdt.pm_naples_ddot_512G /usr/lpp/htx/ecg/ecg.pm_naples_ddot_512G

# MDT for the memex ddot covering 1024G

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_naples_ddot_1024G

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_naples_ddot_1024G
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_naples_ddot_1024G
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_naples_ddot_1024G
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_naples_ddot_1024G
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_naples_ddot_1024G
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_naples_ddot_1024G
echo $opt  '\treg_rules = "hxepowermixer/powerMixer_p8_1024G_memex_ddot.txt" * reg'                      >> ${HTXMDT}mdt.pm_naples_ddot_1024G

ln -sf /usr/lpp/htx/mdt/mdt.pm_naples_ddot_1024G /usr/lpp/htx/ecg/ecg.pm_naples_ddot_1024G

cat ${HTXMDT}mdt.bu | create_mdt_with_devices.awk                              > ${HTXMDT}mdt.pm_p8_switching_cmp

echo $opt                                                                                               >> ${HTXMDT}mdt.pm_p8_switching_cmp
echo $opt  'pwrmxr:'                                                                                    >> ${HTXMDT}mdt.pm_p8_switching_cmp
echo $opt  '\tHE_name = "hxepowermixer"         * Hardware Exerciser name, 14 char'                     >> ${HTXMDT}mdt.pm_p8_switching_cmp
echo $opt  '\tadapt_desc = "CPU-Power"          * adapter description, 11 char max.'                    >> ${HTXMDT}mdt.pm_p8_switching_cmp
echo $opt  '\tdevice_desc = "CPU-Power"         * device description, 15 char max.'                     >> ${HTXMDT}mdt.pm_p8_switching_cmp
echo $opt  '\tcont_on_err = "NO"                * continue on error (YES/NO)'                           >> ${HTXMDT}mdt.pm_p8_switching_cmp
echo $opt  '\treg_rules = "hxepowermixer/powerMixer_p8_swicthing_cmp.txt" * reg'                      >> ${HTXMDT}mdt.pm_p8_switching_cmp

ln -sf /usr/lpp/htx/mdt/mdt.pm_p8_switching_cmp /usr/lpp/htx/ecg/ecg.pm_p8_switching_cmp














