#!/bin/awk -f

# @(#)44	1.5.3.7  src/htx/usr/lpp/htx/etc/scripts/create_shift_sctu_stanzas.awk, htxconf, htxubuntu 10/29/13 05:50:55


BEGIN {

   ncpus = snarf("cat /proc/cpuinfo | grep POWER | cut -d : -f 2 | wc -l");
   
# Determine SMT threads per processor
   lcpus = snarf("cat /proc/cpuinfo | grep processor | wc -l");
   pcpus = snarf("ls -l /proc/device-tree/cpus | grep ^d | awk '($NF ~ /POWER/)' | wc -l");
   
   smt_threads = int(lcpus/pcpus);

   if(ARGV[1] ~ /MR/ ) {
 
 	x=0;
        CpuSet = 16 * smt_threads;
	   for(i=0; i<ncpus;i++) {
             l=i%16;
	       if (l < 4) {	
	        b = int(i / CpuSet);
                k = (b * smt_threads) + (i % (smt_threads));
		sct_dev=sprintf("sctu%s",x);
          	sct_server=sprintf("hxesct%s",k);
          	x++;
		mkstanza(sct_server,"processor","coherence_test",sct_dev,"sct2","rules.InterNode","rules.InterNode");
          }
  	}
   } else if(ARGV[1] ~ /MEMNEST/ )  {

    sctu_num=0;
	server_num = 0;
	gangsize = 16;
	j = 0;
	current_gangnum = -1;
	CpuSet = 16 * smt_threads;
	for ( i = 0 ; i < ncpus ; i++ ) {
	
		if ( i % (CpuSet)  == 0) {
			current_gangnum++;
			base_server_num = current_gangnum * smt_threads;
		}

		current_cpu_in_gang = i % gangsize;

		if ( i % ( smt_threads * 4 ) < (smt_threads/2) ) {
			server_num = server_num + base_server_num;
			progname=sprintf("hxesct%d",server_num);
        		sct_lpn=sprintf("sctu%d", sctu_num);
			sct_rules = "rules.64";
	        	mkstanza(progname,"processor","coherence_test",sct_lpn,"sct2",sct_rules,"rules");
			sctu_num++;
		}

		if ( i % ( smt_threads ) < smt_threads ) {
			server_num = (server_num + 2) % smt_threads;
		}

	}


  } else {
   
        for(i=0; i<ncpus;i++) {
        	j=int(i/16);
        	sct_dev=sprintf("sctu%s",i);
        	sct_server=sprintf("hxesct%s",j);
		mkstanza(sct_server,"processor","coherence_test",sct_dev,"sct2","rules.seq","rules.seq");
        }
  }        
}


function HE_name(x)
    { string_stanza("HE_name",x,"Hardware Exerciser name, 14 char"); }

# use mkstanza() to generate the basic stanza :

function mkstanza(hxe,a,d,dev,rfdir,reg,emc)
{
  # for 4.1, rules file directory is same as exerciser name
    rfdir = hxe

  # make device name entry
    if (create_tlbie_rules) {printf("\n") >> tlbie_mdtfile;}
        else { printf("\n") }
        if (create_tlbie_rules) {
        printf("%s:\n",dev)>> tlbie_mdtfile;
        }
        else {
                printf("%s:\n",dev);
        }

  # make exerciser entry
    if (he[dev]) {
        HE_name(he[dev]);
      # for 3.2, rules file directory is exerciser name minus leading "hxe"
        rfdir = he[dev];
        sub(/^hxe/,"",rfdir);
      # for 4.1, rules file directory is same as exerciser name
        rfdir = he[dev];
    } else {
        if (hxe) HE_name(hxe);
    }

  # make adapter and device description entries
    if (a) adapt_desc(a)
    if (d) device_desc(d)

  # make rules file entries
    if (rf[dev]) {
        string_stanza("reg_rules",sprintf("%s/%s",rfdir,rf[dev]),"reg");
        string_stanza("emc_rules",sprintf("%s/%s",rfdir,rf[dev]),"emc");
    } else {
        if (reg) string_stanza("reg_rules",sprintf("%s/%s",rfdir,reg),"reg");
        if (emc) string_stanza("emc_rules",sprintf("%s/%s",rfdir,emc),"emc");
    }

  # make start_halted entry
    if (flag[dev] == "-h")
        string_stanza("start_halted","y","exerciser halted at startup");
}


function snarf(cmd) {
    snarf_input="";
    cmd | getline snarf_input; close(cmd); return snarf_input;
}

function string_stanza(a,b,c) {
    len=(length(a) + length(b) + 8 + 3 + 2) - 40
        if (create_tlbie_rules) {
        printf("\t%s = \"%s\" %" len "s * %s\n",a,b,"",c) >> tlbie_mdtfile;
        }
        else
    { printf("\t%s = \"%s\" %" len "s * %s\n",a,b,"",c);
        }
}

function adapt_desc(x) { gsub(" ","_",x);
      string_stanza("adapt_desc",x,"adapter description, 11 char max."); }

function device_desc(x) { gsub(" ","_",x);
      string_stanza("device_desc",x,"device description, 15 char max."); }

