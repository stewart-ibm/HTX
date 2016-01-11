// @(#)73	1.2  src/htx/usr/lpp/htx/inc/hxfcpp_wrap.H, htx_libhtx, htxubuntu 6/4/04 14:36:26

extern "C" {
# include <hxihtx.h>
# include <htxlibdef.h>
}

class cHtxLib {
	public:
		cHtxLib (char *pExerName, char *pDevName, char *pRunType);
		void start ();
		void finish ();
		void sendMsg (int, int, char *);
		int hxfohft (int);
		int hxfpat (char *, char *, size_t);
		void setRunType (char *type = "OTH");
		~cHtxLib ();

	private:
		struct htx_data htx_data;
};
