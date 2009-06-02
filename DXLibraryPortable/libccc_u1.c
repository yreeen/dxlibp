int cccSJIStoUCS2(cccUCS2 * dst, size_t count, cccCode const * str) {
	if (!str || !dst) return 0;//NULLチェック
	if (!cccInitialized) cccInit();//初期化チェック
	if (!(__table_ptr__[CCC_CP932])) cccLoadTable("flash0:/vsh/etc/cptbl.dat", CCC_CP932);//変換テーブルのロード
	
	int i = 0, length = 0, j, code, id;
	if (__table_ptr__[CCC_CP932])
	{ //変換テーブルが存在する場合
		unsigned short *header = (unsigned short*)(__table_ptr__[CCC_CP932]);

		cccUCS2 *SJIStoUCS2 = (cccUCS2*)header+header[2]*3+3;		
		while (str[i] && length < count)
		{
			code = str[i];
			id = -1;
			for (j = 1; (j <= header[2]) && (id < 0); j++)
			{
				if ((code >= header[j*3]) && (code <= header[j*3+1]))
				{
					id = header[j*3+2] + code - header[j*3]; 
				} else {
					if (j == 2) code = 0x0200 * str[i] - 0xE100 - ((str[i] >= 0xE0) ? 0x8000 : 0) + str[i+1] + ((str[i+1] <= 0x7E) ? -0x1F : ((str[i+1] >= 0x9F) ? 0x82 : -0x20) );
				}
			}
			dst[length++] = (id < 0) ? __error_char_ucs2__ : SJIStoUCS2[id];
			i += (str[i] <= 0x80 || (str[i] >= 0xA0 && str[i] <= 0xDF) || str[i] >= 0xFD) ? 1 : 2; //single or double byte
		}
	} else { //変換テーブルが存在しない場合
		while (str[i] && length < count) {
			dst[length++] = __error_char_ucs2__;
			i += (str[i] <= 0x80 || (str[i] >= 0xA0 && str[i] <= 0xDF) || str[i] >= 0xFD) ? 1 : 2; //single or double byte
		}
	}
	return length;
}
