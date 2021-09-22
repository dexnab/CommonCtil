/**
* @file cmtCore.c
* @date 2021-09-22
* @author Dexnab
*/

#include <cmtString.h>

cmtUint8 cmtANSIchSize(cmtChar* ch, cmtChar* locale)
{
	cmtUint8 chsize;
	cmtChar CurLocaleCp[CMT_LOCALE_MAX], * CurLocale;

	//保存当前locale
	/*调试笔记:
	* 之后setlocale会改变此处返回值指向的内存,所以需要复制一遍
	*/
	memset(CurLocaleCp, 0, sizeof(CurLocaleCp));
	CurLocale = setlocale(LC_ALL, NULL);
	strncpy(CurLocaleCp, CurLocale, sizeof(CurLocaleCp) - 1);
	//设置locale
	setlocale(LC_ALL, locale);

	chsize = mblen(ch, MB_CUR_MAX);

	//恢复locale
	setlocale(LC_ALL, CurLocaleCp);

	return chsize;
}

cmtUint64 cmtANSIstrSize(cmtChar* str)
{
	cmtUint64 size = 0;

	while (str[size]) size++;

	return size;
}

cmtUint64 cmtANSIlen(cmtANSIstr* str)
{
	cmtUint64 len = 0, r = 0;
	cmtChar CurLocaleCp[CMT_LOCALE_MAX], * CurLocale;

	//保存当前locale
	memset(CurLocaleCp, 0, sizeof(CurLocaleCp));
	CurLocale = setlocale(LC_ALL, NULL);
	strncpy(CurLocaleCp, CurLocale, sizeof(CurLocaleCp) - 1);
	//设置locale
	setlocale(LC_ALL, str->locale);

	while (r < str->size)
	{
		//mblen不会把'\0'算作字符，所以需要单独来处理
		if (!str->data[r]) r++;
		else r += mblen(str->data + r, str->size - r);
		len++;
	}

	//恢复locale
	setlocale(LC_ALL, CurLocaleCp);

	return len;
}

cmtUint64 cmtANSItoU8size(cmtANSIstr* ansi)
{
	cmtUint64 rAs = 0, u8size = 0;
	cmtUint8 chsize;
	cmtWchar u16temp[2];
	cmtChar CurLocaleCp[CMT_LOCALE_MAX], * CurLocale;

	//保存当前locale
	memset(CurLocaleCp, 0, sizeof(CurLocaleCp));
	CurLocale = setlocale(LC_ALL, NULL);
	strncpy(CurLocaleCp, CurLocale, sizeof(CurLocaleCp) - 1);
	//设置locale
	setlocale(LC_ALL, ansi->locale);

	while (rAs < ansi->size)
	{
		//'\0'
		if (!ansi->data[rAs])
		{
			rAs++;
			u8size++;
			continue;
		}

		chsize = mblen(ansi->data + rAs, ansi->size - rAs);
		mbtowc(u16temp, ansi->data + rAs, chsize);
		//如果首字在保留区间外，那么绝对只有一个字
		if (u16temp[0] < CMT_UNICODE_RSV_START || u16temp[0] > CMT_UNICODE_RSV_END)
		{
			//[0,0x7f]
			if (u16temp[1] < 0x80) u8size += 1;
			//[0x80,0x07ff]
			else if (u16temp[1] < 0x800) u8size += 2;
			//[0x0800,0xffff]
			else u8size += 3;
		}
		//如果首字在保留区间内，那么绝对有两个字，范围为[0x010000,0x10ffff]
		else u8size += 4;
		rAs += chsize;
	}

	//恢复locale
	setlocale(LC_ALL, CurLocaleCp);

	return u8size;
}

void cmtANSItoU8(cmtANSIstr* ansi, cmtU8str* u8)
{
	cmtUint64 rAs = 0, rU8 = 0;
	cmtUint8 chsize;
	cmtWchar u16temp[2];
	cmtChar CurLocaleCp[CMT_LOCALE_MAX], * CurLocale;

	//保存当前locale
	memset(CurLocaleCp, 0, sizeof(CurLocaleCp));
	CurLocale = setlocale(LC_ALL, NULL);
	strncpy(CurLocaleCp, CurLocale, sizeof(CurLocaleCp) - 1);
	//设置locale
	setlocale(LC_ALL, ansi->locale);

	while (rAs < ansi->size)
	{
		//'\0'
		if (!ansi->data[rAs])
		{
			u8->data[rU8] = 0;
			rAs++;
			rU8++;
			continue;
		}

		chsize = mblen(ansi->data + rAs, ansi->size - rAs);
		mbtowc(u16temp, ansi->data + rAs, chsize);
		//如果首字在保留区间外，那么绝对只有一个字
		if (u16temp[0] < CMT_UNICODE_RSV_START || u16temp[0] > CMT_UNICODE_RSV_END)
		{
			//[0,0x7f]
			if (u16temp[1] < 0x80)
			{
				u8->data[rU8] = (cmtUint8)u16temp[1];
				rU8 += 1;
			}
			//[0x80,0x07ff]
			else if (u16temp[1] < 0x800)
			{
				//u8第二字节
				u8->data[rU8 + 1] = 0x80 + (cmtUint8)u16temp[1] & 0x3f;
				//u8第一字节
				u16temp[1] >>= 6;
				u8->data[rU8] = 0xc0 + (cmtUint8)u16temp[1];
				rU8 += 2;
			}
			//[0x0800,0xffff]
			else
			{
				//u8第三字节
				u8->data[rU8 + 2] = 0x80 + (cmtUint8)u16temp[1] & 0x3f;
				//u8第二字节
				u16temp[1] >>= 6;
				u8->data[rU8 + 1] = 0x80 + (cmtUint8)u16temp[1] & 0x3f;
				//u8第三字节
				u16temp[1] >>= 6;
				u8->data[rU8] = 0xe0 + (cmtUint8)u16temp[1];
				rU8 += 3;
			}
		}
		//如果首字在保留区间内，那么绝对有两个字，范围为[0x010000,0x10ffff]
		else
		{
			u16temp[0] = u16temp[0] - 0xd800 + 0x40;
			u16temp[1] -= 0xdc00;
			//u8第四字节
			u8->data[rU8 + 3] = 0x80 + (cmtUint8)u16temp[1] & 0x3f;
			//u8第三字节
			u16temp[1] >>= 6;
			u8->data[rU8 + 2] = 0x80 + (cmtUint8)u16temp[1] + (((cmtUint8)u16temp[0] & 0x3) << 4);
			//u8第二字节
			u16temp[0] >>= 2;
			u8->data[rU8 + 1] = 0x80 + (cmtUint8)u16temp[0] & 0x3f;
			//u8第一字节
			u16temp[0] >>= 6;
			u8->data[rU8] = 0xf0 + (cmtUint8)u16temp[0];
			rU8 += 4;
		}
		rAs += chsize;
	}

	//恢复locale
	setlocale(LC_ALL, CurLocaleCp);
}

cmtUint64 cmtANSItoU16size(cmtANSIstr* ansi)
{
	cmtUint64 rAs = 0, u16size = 0;
	cmtUint8 chsize;
	cmtWchar u16temp[2];
	cmtChar CurLocaleCp[CMT_LOCALE_MAX], * CurLocale;

	//保存当前locale
	memset(CurLocaleCp, 0, sizeof(CurLocaleCp));
	CurLocale = setlocale(LC_ALL, NULL);
	strncpy(CurLocaleCp, CurLocale, sizeof(CurLocaleCp) - 1);
	//设置locale
	setlocale(LC_ALL, ansi->locale);

	while (rAs < ansi->size)
	{
		//'\0'
		if (!ansi->data[rAs])
		{
			rAs++;
			u16size += 2;
			continue;
		}

		chsize = mblen(ansi->data + rAs, ansi->size - rAs);
		mbtowc(u16temp, ansi->data + rAs, chsize);
		//如果首字在保留区间外，那么绝对只有一个字
		if (u16temp[0] < CMT_UNICODE_RSV_START || u16temp[0] > CMT_UNICODE_RSV_END) u16size += 2;
		//如果首字在保留区间内，那么绝对有两个字，范围为[0x010000,0x10ffff]
		else u16size += 4;
		rAs += chsize;
	}

	//恢复locale
	setlocale(LC_ALL, CurLocaleCp);

	return u16size;
}

void cmtANSItoU16(cmtANSIstr* ansi, cmtU16str* u16)
{
	cmtUint64 rAs = 0, rU16 = 0;
	cmtUint8 chsize;
	cmtChar CurLocaleCp[CMT_LOCALE_MAX], * CurLocale;

	//保存当前locale
	memset(CurLocaleCp, 0, sizeof(CurLocaleCp));
	CurLocale = setlocale(LC_ALL, NULL);
	strncpy(CurLocaleCp, CurLocale, sizeof(CurLocaleCp) - 1);
	//设置locale
	setlocale(LC_ALL, ansi->locale);

	while (rAs < ansi->size)
	{
		//'\0'
		if (!ansi->data[rAs])
		{
			u16->data[rU16] = 0;
			rAs++;
			rU16++;
			continue;
		}

		chsize = mblen(ansi->data + rAs, ansi->size - rAs);
		mbtowc(u16->data + rU16, ansi->data + rAs, chsize);
		//如果首字在保留区间外，那么绝对只有一个字
		if (u16->data[rU16] < CMT_UNICODE_RSV_START || u16->data[rU16] > CMT_UNICODE_RSV_END) rU16++;
		//如果首字在保留区间内，那么绝对有两个字，范围为[0x010000,0x10ffff]
		else rU16 += 2;
		rAs += chsize;
	}

	//恢复locale
	setlocale(LC_ALL, CurLocaleCp);
}

cmtUint64 cmtANSItoU32size(cmtANSIstr* ansi)
{
	return cmtANSIlen(ansi) * 4;
}

void cmtANSItoU32(cmtANSIstr* ansi, cmtU32str* u32)
{
	cmtUint64 rAs = 0, rU32 = 0;
	cmtUint8 chsize;
	cmtWchar u16temp[2];
	cmtChar CurLocaleCp[CMT_LOCALE_MAX], * CurLocale;

	//保存当前locale
	memset(CurLocaleCp, 0, sizeof(CurLocaleCp));
	CurLocale = setlocale(LC_ALL, NULL);
	strncpy(CurLocaleCp, CurLocale, sizeof(CurLocaleCp) - 1);
	//设置locale
	setlocale(LC_ALL, ansi->locale);

	while (rAs < ansi->size)
	{
		//'\0'
		if (!ansi->data[rAs])
		{
			u32->data[rU32] = 0;
			rAs++;
			rU32++;
			continue;
		}

		chsize = mblen(ansi->data + rAs, ansi->size - rAs);
		mbtowc(u16temp, ansi->data + rAs, chsize);
		//如果首字在保留区间外，那么绝对只有一个字
		if (u16temp[0] < CMT_UNICODE_RSV_START || u16temp[0] > CMT_UNICODE_RSV_END)
		{
			u32->data[rU32] = u16temp[1];
			rU32++;
		}
		//如果首字在保留区间内，那么绝对有两个字，范围为[0x010000,0x10ffff]
		else
		{
			u16temp[0] = u16temp[0] - 0xd800 + 0x40;
			u16temp[1] -= 0xdc00;
			u32->data[rU32] = (u16temp[0] << 16) + u16temp[1];
			rU32 += 2;
		}
		rAs += chsize;
	}

	//恢复locale
	setlocale(LC_ALL, CurLocaleCp);
}

cmtUint8 cmtU8chSize(cmtChar* ch)
{
	if (*ch < 0x80) return 1;
	if (*ch < 0xe0) return 2;
	if (*ch < 0xf0) return 3;
	return 4;
}

cmtUint64 cmtU8strSize(cmtChar* str)
{
	cmtUint64 size = 0;

	while (str[size]) size++;

	return size;
}

cmtUint64 cmtU8len(cmtU8str* str)
{
	cmtUint64 r = 0, len = 0;

	while (r < str->size)
		if (str->data[r] < 0x80 || str->data[r] >= 0xc0) len++;

	return len;
}

cmtUint64 cmtU8toANSIsize(cmtU8str* u8, cmtChar* locale, cmtBool* err)
{
	cmtUint64 rU8 = 0, ASsize = 0;
	cmtInt8 chsize;
	cmtWchar u16temp[2];
	cmtChar AStemp[MB_LEN_MAX];
	cmtChar CurLocaleCp[CMT_LOCALE_MAX], * CurLocale;

	//保存当前locale
	memset(CurLocaleCp, 0, sizeof(CurLocaleCp));
	CurLocale = setlocale(LC_ALL, NULL);
	strncpy(CurLocaleCp, CurLocale, sizeof(CurLocaleCp) - 1);
	//设置locale
	setlocale(LC_ALL, locale);

	while (rU8 < u8->size)
	{
		//'\0'
		if (!u8->data[rU8])
		{
			rU8++;
			ASsize++;
			continue;
		}

		if (u8->data[rU8] < 0x80)
		{
			u16temp[0] = u8->data[rU8] & 0x7f;
			rU8++;
		}
		else if (u8->data[rU8] < 0xe0)
		{
			u16temp[0] = u8->data[rU8] & 0x1f;
			u16temp[0] <<= 6;
			u16temp[0] += u8->data[rU8 + 1] & 0x3f;
			rU8 += 2;
		}
		else if (u8->data[rU8] < 0xf0)
		{
			u16temp[0] = u8->data[rU8] & 0xf;
			u16temp[0] <<= 6;
			u16temp[0] += u8->data[rU8 + 1] & 0x3f;
			u16temp[0] <<= 6;
			u16temp[0] += u8->data[rU8 + 2] & 0x3f;
			rU8 += 3;
		}
		else
		{
			u16temp[0] = u8->data[rU8] & 0x7;
			u16temp[0] <<= 6;
			u16temp[0] += u8->data[rU8 + 1] & 0x3f;
			u16temp[0] <<= 2;
			u16temp[0] += u8->data[rU8 + 2] >> 4 & 0x3;
			u16temp[0] = u16temp[0] - 0x40 + 0xd800;
			u16temp[1] = u8->data[rU8 + 2] & 0xf;
			u16temp[1] <<= 6;
			u16temp[1] += u8->data[rU8 + 3] & 0x3f;
			u16temp[1] += 0xdc00;
			rU8 += 4;
		}

		chsize = wctomb(AStemp, *u16temp);
		if (chsize == -1)
		{
			//恢复locale
			setlocale(LC_ALL, CurLocaleCp);

			*err = TRUE;
			return 0;
		}

		ASsize += chsize;
	}

	//恢复locale
	setlocale(LC_ALL, CurLocaleCp);

	*err = FALSE;
	return ASsize;
}

cmtBool cmtU8toANSI(cmtU8str* u8, cmtANSIstr* ansi)
{
	cmtUint64 rU8 = 0, rAs = 0;
	cmtInt8 chsize;
	cmtWchar u16temp[2];
	cmtChar CurLocaleCp[CMT_LOCALE_MAX], * CurLocale;

	//保存当前locale
	memset(CurLocaleCp, 0, sizeof(CurLocaleCp));
	CurLocale = setlocale(LC_ALL, NULL);
	strncpy(CurLocaleCp, CurLocale, sizeof(CurLocaleCp) - 1);
	//设置locale
	setlocale(LC_ALL, ansi->locale);

	while (rU8 < u8->size)
	{
		//'\0'
		if (!u8->data[rU8])
		{
			ansi->data[rAs] = 0;
			rU8++;
			rAs++;
			continue;
		}

		if (u8->data[rU8] < 0x80)
		{
			u16temp[0] = u8->data[rU8] & 0x7f;
			rU8++;
		}
		else if (u8->data[rU8] < 0xe0)
		{
			u16temp[0] = u8->data[rU8] & 0x1f;
			u16temp[0] <<= 6;
			u16temp[0] += u8->data[rU8 + 1] & 0x3f;
			rU8 += 2;
		}
		else if (u8->data[rU8] < 0xf0)
		{
			u16temp[0] = u8->data[rU8] & 0xf;
			u16temp[0] <<= 6;
			u16temp[0] += u8->data[rU8 + 1] & 0x3f;
			u16temp[0] <<= 6;
			u16temp[0] += u8->data[rU8 + 2] & 0x3f;
			rU8 += 3;
		}
		else
		{
			u16temp[0] = u8->data[rU8] & 0x7;
			u16temp[0] <<= 6;
			u16temp[0] += u8->data[rU8 + 1] & 0x3f;
			u16temp[0] <<= 2;
			u16temp[0] += u8->data[rU8 + 2] >> 4 & 0x3;
			u16temp[0] = u16temp[0] - 0x40 + 0xd800;
			u16temp[1] = u8->data[rU8 + 2] & 0xf;
			u16temp[1] <<= 6;
			u16temp[1] += u8->data[rU8 + 3] & 0x3f;
			u16temp[1] += 0xdc00;
			rU8 += 4;
		}

		chsize = wctomb(ansi->data + rAs, *u16temp);
		if (chsize == -1)
		{
			//恢复locale
			setlocale(LC_ALL, CurLocaleCp);

			return TRUE;
		}

		rAs += chsize;
	}

	//恢复locale
	setlocale(LC_ALL, CurLocaleCp);

	return FALSE;
}

cmtUint64 cmtU8toU16size(cmtU8str* u8)
{
	cmtUint64 rU8 = 0, u16size = 0;

	while (rU8 < u8->size)
	{
		//'\0'
		if (!u8->data[rU8])
		{
			rU8++;
			u16size += 2;
			continue;
		}

		if (u8->data[rU8] < 0x80)
		{
			u16size += 2;
			rU8++;
		}
		else if (u8->data[rU8] < 0xe0)
		{
			u16size += 2;
			rU8 += 2;
		}
		else if (u8->data[rU8] < 0xf0)
		{
			u16size += 2;
			rU8 += 3;
		}
		else
		{
			u16size += 4;
			rU8 += 4;
		}
	}

	return u16size;
}

void cmtU8toU16(cmtU8str* u8, cmtU16str* u16)
{
	cmtUint64 rU8 = 0, rU16 = 0;
	cmtWchar u16temp[2];

	while (rU8 < u8->size)
	{
		//'\0'
		if (!u8->data[rU8])
		{
			u16->data[rU16] = 0;
			rU8++;
			rU16++;
			continue;
		}

		if (u8->data[rU8] < 0x80)
		{
			u16->data[rU16] = u8->data[rU8] & 0x7f;
			rU8++;
			rU16++;
		}
		else if (u8->data[rU8] < 0xe0)
		{
			u16temp[0] = u8->data[rU8] & 0x1f;
			u16temp[0] <<= 6;
			u16temp[0] += u8->data[rU8 + 1] & 0x3f;
			u16->data[rU16] = u16temp[0];
			rU8 += 2;
			rU16++;
		}
		else if (u8->data[rU8] < 0xf0)
		{
			u16temp[0] = u8->data[rU8] & 0xf;
			u16temp[0] <<= 6;
			u16temp[0] += u8->data[rU8 + 1] & 0x3f;
			u16temp[0] <<= 6;
			u16temp[0] += u8->data[rU8 + 2] & 0x3f;
			u16->data[rU16] = u16temp[0];
			rU8 += 3;
			rU16++;
		}
		else
		{
			u16temp[0] = u8->data[rU8] & 0x7;
			u16temp[0] <<= 6;
			u16temp[0] += u8->data[rU8 + 1] & 0x3f;
			u16temp[0] <<= 2;
			u16temp[0] += u8->data[rU8 + 2] >> 4 & 0x3;
			u16temp[0] = u16temp[0] - 0x40 + 0xd800;
			u16temp[1] = u8->data[rU8 + 2] & 0xf;
			u16temp[1] <<= 6;
			u16temp[1] += u8->data[rU8 + 3] & 0x3f;
			u16temp[1] += 0xdc00;
			u16->data[rU16] = u16temp[0];
			u16->data[rU16 + 1] = u16temp[1];
			rU8 += 4;
			rU16 += 2;
		}
	}
}