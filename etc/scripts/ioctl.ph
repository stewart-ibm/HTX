
# @(#)49        1.1     src/htx/usr/lpp/htx/etc/scripts/ioctl.ph, htxconf, htxubuntu 3/23/05 03:52:40

# This perl-header file is used by ioctl's used in perl script '/usr/lpp/htx/etc/scripts/part'
# ioctl's using this perl-header file are
# BLKGETSIZE
# BLKSSZGET
# This perl-header is created by using h2ph utility i.e. running h2ph on dedicated C-header
# file for above ioctl's

# Equivalent of definitions in 'linux/fs.h' 

# returns device size 512 (long *arg)

sub BLKGETSIZE () { &_IO(0x12,96); }

# gets block device sector size

sub BLKSSZGET () { &_IO(0x12,104); }

# Equivalent of definitions in 'asm-ppc64/ioctl.h'

sub _IO {
	local($type,$nr) = @_;
	eval q( &_IOC( &_IOC_NONE,($type),($nr),0));
}

sub _IOC {
	local($dir,$type,$nr,$size) = @_;
	eval q(((($dir) <<  &_IOC_DIRSHIFT) | (($type) <<  &_IOC_TYPESHIFT) | (($nr) <<  &_IOC_NRSHIFT) | (($size) <<  &_IOC_SIZESHIFT)));
}

sub _IOC_DIRSHIFT () { ( &_IOC_SIZESHIFT+ &_IOC_SIZEBITS); }

sub _IOC_TYPESHIFT () {	( &_IOC_NRSHIFT+ &_IOC_NRBITS); }

sub _IOC_NRSHIFT () { 0; }

sub _IOC_SIZESHIFT () {	( &_IOC_TYPESHIFT+ &_IOC_TYPEBITS); }

sub _IOC_NONE () { 1; }

sub _IOC_SIZEBITS () { 13; }
    
sub _IOC_NRBITS () { 8; }

sub _IOC_TYPEBITS () { 8; }

1;
