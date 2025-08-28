// ==========================================================================
//code=UTF8	tab=4
//
// Lutino:	Application SErver.
//
// 		ltn_tools.cpp
//		$Revision: 1.0 $
//		$Date: 2018/02/12 21:11:00 $
//
// ==========================================================================
//---------------------------------------------------------------------------
#define _CRT_SECURE_NO_WARNINGS
#include  <ctype.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <stdarg.h>
#include  <string.h>
#include  <limits.h>
#include  <errno.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <time.h>

#ifdef linux
#include  <fcntl.h>
#include  <unistd.h>
#include  <sys/time.h>
#include  <sys/socket.h>
#include  <sys/un.h>
#include  <netdb.h>
#include  <pthread.h>
#else
#include  <errno.h>
#include  <io.h>
#include  <process.h>
#endif
//#include  "const.h"
#include "ltn_tools.h"
#include "define.h"
//static char debug_log_filename[FILENAME_MAX];	// デバッグログ出力ファイル名(フルパス)
static char debug_log_initialize_flag = (1);	// デバッグログ初期化フラグ
static void cut_before_n_length(char* sentence, unsigned int n);
static void cut_after_n_length(char* sentence, unsigned int n);

/// <summary>
/// CRLFCRLFのパターンを見つけて文字列化
/// </summary>
/// <param name="start">開始ポインタ</param>
/// <param name="end">終端ポインタ</param>
/// <returns>パターンのエンドポイント</returns>
char* seekCRLFCRLF(char* start, char* end)
{
	while (start <= end - 4)
	{
		char* nptr = start;
		if (*start++ == '\r')
		{
			if (*start++ == '\n')
			{
				if (*start++ == '\r')
				{
					if (*start++ == '\n')
					{
						*nptr = 0;
						return start;
					}
				}
			}
		}
	}
	return NULL;
}

/// <summary>
/// 部分文字列の位置を示す  
/// </summary>
/// <param name="haystack">全データ</param>
/// <param name="haystacklen">全データ長</param>
/// <param name="needle">探索データ</param>
/// <param name="needlelen">探索データ長さ</param>
/// <returns></returns>
void* memmem(const void* haystack, size_t haystacklen, const void* needle, size_t needlelen)
{
	const char* begin = static_cast<const char*>(haystack);
	const char* last = begin + haystacklen - needlelen;

	if (needlelen == 0) {
		return (void*)begin;
	}

	for (; begin < last; begin++) {
		/// １文字検査して合致するならメモリブロック一致検査
		if (*begin == *static_cast<const char*>(needle) && memcmp(begin, needle, needlelen) == 0) {
			return (void*)begin;
		}
	}

	return NULL;
}
/// <summary>
/// 最初に出て来たcut_charの前後を分割
/// </summary>
/// <param name="pattern">切り出し対象文字列</param>
/// <param name="split1">前の文字列</param>
/// <param name="split2">後の文字列&元の文字列</param>
/// <returns>成功:true/失敗:false</returns>
bool split(const char* pattern, wString& split1, wString& split2)
{
	int pos = split2.Pos(pattern);
	if (pos == wString::npos) {
		split1 = split2;
		split2.clear();
		return false;
	}
	else {
		split1 = split2.substr(0, pos);
		split2 = split2.substr(pos + 1);
		return true;
	}
}

/// <summary>
/// sentence文字列内のkey文字列をrep文字列で置換する。
/// </summary>
/// <param name="sentence">置換元文字列</param>
/// <param name="key">置換文字列</param>
/// <param name="rep">置換後の文字列</param>
void replace_character(char* sentence, const char* key, const char* rep)
{
	int klen = (int)strlen(key);
	int rlen = (int)strlen(rep);
	int slen = (int)strlen(sentence);
	int num;
	if (klen == 0 || slen == 0) {
		return;
	}
	auto p = strstr(sentence, key);
	if (klen == rlen) {
		//前詰め置換そのままコピーすればいい
		while (p != NULL) {
			memcpy(static_cast<void*>(p), static_cast<const void*>(rep), rlen);
			p = strstr(p + rlen, key);
		}
	}
	else if (klen > rlen) {
		num = klen - rlen;
		while (p != NULL) {
			auto q = p;
			while (1) {
				*q = *(q + num);
				if (*(q + num) == 0) {
					break;
				}
				q++;
			}
			memcpy(p, rep, rlen);
			p = strstr(p + rlen, key);
			//slen -= num;
		}
		//置換文字が長いので後詰めする
	}
	else {
		while (p != NULL) {
			num = rlen - klen;
			//pからrlen-klenだけのばす
			for (auto str = sentence + slen + num; str - num >= p; str--) {
				*str = *(str - num);
			}
			memcpy(p, rep, rlen);
			p = strstr(p + rlen, key);
			slen += num;
		}
	}
	//myfree(buf);
	return;
}
/// <summary>
/// entence文字列内の最初のkey文字列をrep文字列で置換する。
/// </summary>
/// <param name="sentence">対象文字列</param>
/// <param name="key">置換元文字</param>
/// <param name="rep">置換先文字</param>
void replace_character_first(char* sentence, const char* key, const char* rep)
{
	auto klen = (int)strlen(key);
	auto rlen = (int)strlen(rep);
	auto slen = (int)strlen(sentence);
	int num;
	if (klen == 0 || slen == 0) {
		return;
	}
	auto p = strstr(sentence, key);
	if (p == 0)
	{
		return;
	}
	if (klen == rlen) {
		memcpy(p, rep, rlen);
		//前詰め置換そのままコピーすればいい
	}
	else if (klen > rlen)
	{
		num = klen - rlen;
		strcpy(p, (p + num));
		memcpy(p, rep, rlen);
		//置換文字が長いので後詰めする
	}
	else
	{
		num = rlen - klen;
		//pからrlen-klenだけのばす
		for (auto str = sentence + slen + num; str - num >= p; str--) {
			*str = *(str - num);
		}
		memcpy(p, rep, rlen);
	}
	return;
}
//***************************************************************************
// sentence文字列より、cut_charから後ろを削除
//      見つからなければ何もしない。
// 入力  : char* sentence 入力文字列
//         char  cut_char 切り取り文字
// 戻り値: char* 切り取った後ろの文字列
//***************************************************************************
char* cut_after_character(char* sentence, char cut_char)
{
	char* symbol_p;
	// 削除対象キャラクターがあった場合、それから後ろを削除。
	symbol_p = strchr(sentence, cut_char);
	if (symbol_p != NULL) {
		*symbol_p++ = '\0';
	}
	return symbol_p;
}
//************************************************************************
// sentence文字列の、cut_charが最後に出てきた所から後ろをCUT
// もし、cut_charがsentence文字列に入っていなかった場合、文字列全部削除。
//************************************************************************
void 	cut_after_last_character(char* sentence, char cut_char)
{
	char* symbol_p;
	if (sentence == NULL || *sentence == 0) {
		return;
	}
	// 削除対象キャラクターが最後に出てくる所を探す。
	symbol_p = strrchr(sentence, cut_char);
	if (symbol_p == NULL) {
		// 発見できなかった場合、文字列全部削除。
		*sentence = '\0';
		return;
	}
	*symbol_p = '\0';
}

/// <summary>
/// sentenceの、後ろ n byteを残して削除。
/// </summary>
/// <param name="sentence">対象文字列</param>
/// <param name="n">バイト数</param>
void    cut_before_n_length(char* sentence, unsigned int n)
{
	if (sentence == NULL || *sentence == 0) {
		return;
	}
	auto sentence_len = (unsigned int)strlen(sentence);
	// sentence が、nよりも同じか短いならばreturn
	if (sentence_len <= n) {
		return;
	}
	// テンポラリエリアmalloc.
	auto malloc_p = static_cast<char*>(malloc(sentence_len + 10));
	if (malloc_p == NULL) {
		return;
	}
	auto work_p = sentence;
	work_p += sentence_len;
	work_p -= n;
	strncpy(malloc_p, work_p, sentence_len + 10);
	strncpy(sentence, malloc_p, sentence_len);
	free(malloc_p);
	return;
}
/// <summary>
/// sentenceの、後ろ n byteを削除
/// 全長がn byteに満たなかったら、文字列全部削除
/// </summary>
/// <param name="sentence"></param>
/// <param name="n"></param>
void    cut_after_n_length(char* sentence, unsigned int n)
{
	if (sentence == NULL || *sentence == 0) {
		return;
	}
	auto sentence_len = (unsigned int)strlen(sentence);
	// sentence が、nよりも同じか短いならば、全部削除してreturn;
	if (sentence_len <= n) {
		*sentence = 0;
		//strncpy(sentence, "", sentence_len);
		return;
	}
	// 後ろ n byteを削除
	auto work_p = sentence;
	work_p += sentence_len;
	work_p -= n;
	*work_p = '\0';
	return;
}
/// <summary>
/// sentence文字列の、cut_charを抜く。
/// </summary>
/// <param name="sentence">対象文字列</param>
/// <param name="cut_char">カットする文字</param>
void    cut_character(char* sentence, char cut_char)
{
	char* work1;
	char* work2;
	if (sentence == NULL || *sentence == 0) {
		return;
	}
	work1 = sentence;
	work2 = sentence;
	// 処理ループ。
	while (*work1) {
		// 削除対象のキャラクターがいたら、それを飛ばす。
		if (*work1 == cut_char) {
			work1++;
		}
		else {
			*work2++ = *work1++;
		}
	}
	// '\0' をコピー。
	*work2 = '\0';
	return;
}
/// <summary>
/// 文字列の左端文字削除
/// </summary>
/// <param name="sentence">対象文字列</param>
/// <param name="cut_char">削除文字（省略値=' '）</param>
void ltrim(char* sentence, char cut_char)
{
	char* p = sentence;
	if (sentence == NULL || *sentence == 0) {
		return;
	}
	// 削除対象キャラクターがあるかぎり進める。
	while ((*p == cut_char) && *p) {
		p++;
	}
	// sentence書き換え
	if (p != sentence) {
		while (*p) {
			*sentence++ = *p++;
		}
		*sentence = 0;
		//string corupped bug.
		//strcpy((char*)sentence, (char*)p);
	}
	return;
}

/// <summary>
/// 文字列の右端文字削除
/// </summary>
/// <param name="sentence">対象文字列</param>
/// <param name="cut_char">削除文字（省略値=' '）</param>
void rtrim(char* sentence, char cut_char)
{
	if (sentence == NULL || *sentence == 0) {
		return;
	}
	auto length = (int)strlen(sentence);  // 文字列長Get
	auto source_p = sentence;
	source_p += length;         // ワークポインタを文字列の最後にセット。
	for (auto i = 0; i < length; i++) {// 文字列の数だけ繰り返し。
		source_p--;                     // 一文字ずつ前へ。
		if (*source_p == cut_char) {// 削除キャラ ヒットした場合削除
			*source_p = '\0';
		}
		else {// 違うキャラが出てきたところで終了。
			break;
		}
	}
	return;
}
/********************************************************************************/
// sentence文字列内のunique_charが連続しているところを、unique_char1文字だけにする。
/********************************************************************************/
void duplex_character_to_unique(char* sentence, char unique_char)
{
	char* p1;
	char* p2;
	int                 unique_char_count = 0;
	if (sentence == NULL || *sentence == 0) {
		return;
	}
	p1 = sentence;
	p2 = sentence;
	// sensense文字列から、unique_char以外をワークへコピー。
	while (*p1) {
		// unique_char発見
		if (*p1 == unique_char) {
			// 最初の一つならコピー。それ以外ならスキップ。
			if (unique_char_count == 0) {
				*p2++ = *p1++;
			}
			else {
				p1++;
			}
			unique_char_count++;
			// unique_char 以外ならコピー。
		}
		else {
			unique_char_count = 0;
			*p2++ = *p1++;
		}
	}
	*p2 = '\0';
	return;
}
//*********************************************************
// sentence文字列より、最初に出て来たcut_charの前後を分割。
//
//      sentence        (IN) 分割対象の文字列を与える。
//      cut_char        (IN) 分割対象の文字を入れる。
//      split1          (OUT)カットされた前の部分が入る。sentenceと同等のサイズが望ましい。
//      split2          (OUT)カットされた後ろの部分が入る。sentenceと同等のサイズが望ましい。
//
//
// return
//              0:                      正常終了。
//              それ以外：      エラー。分割失敗などなど。
//*********************************************************
int sentence_split(char* sentence, char cut_char, char* split1, char* split2)
{
	char* p = sentence;
	char* pos;
	// エラーチェック。
	if (sentence == NULL || *sentence == 0 || split1 == NULL || split2 == NULL) {
		return 1;       // 引数にNULLまじり。
	}
	pos = strchr(sentence, cut_char);
	// 分割文字発見できず。
	if (pos == NULL) {
		return 1;
	}
	//比較しながら複写
	while (*p) {
		//分割文字より前半
		if (p < pos) {
			*split1++ = *p++;
			//分割文字より後半
		}
		else if (p > pos) {
			*split2++ = *p++;
			//分割文字位置
		}
		else {
			p++;
		}
	}
	//糸止め
	*split1 = 0;
	*split2 = 0;
	return 0; // 正常終了。
}
//******************************************************************
// filenameから、拡張子を取り出す('.'も消す）
// '.'が存在しなかった場合、拡張子が長すぎた場合は、""が入る。
//******************************************************************
void filename_to_extension(char* filename, char* extension_buf, unsigned int extension_buf_size)
{
	// 拡張子の存在チェック。
	auto p = strrchr(filename, '.');
	if ((p == NULL) || (strlen(p) > extension_buf_size)) {
		*extension_buf = 0;
		//strncpy(extension_buf, "", extension_buf_size );
		return;
	}
	// 拡張子を切り出し。
	p++;
	strncpy(extension_buf, p, extension_buf_size);
	return;
}
// **************************************************************************
// text_buf から、CR/LF か'\0'が現れるまでを切り出して、line_bufにcopy。
// (CR/LFはcopyされない)
// 次の行の頭のポインタをreturn。
// Errorか'\0'が現れたらNULLが戻る。
// **************************************************************************
//char* buffer_distill_line(char* text_buf_p, char* line_buf_p, unsigned int line_buf_size)
//{
//	char* p;
//	unsigned int                counter = 0;
//	p = text_buf_p;
//	// ------------------
//	// CR/LF '\0'を探す
//	// ------------------
//	while (1) {
//		if (*p == '\r') { // CR
//			p++;
//			continue;
//		}
//		if (*p == '\n') { // LF
//			p++;
//			break;
//		}
//		if (*p == '\0') {
//			break;
//		}
//		p++;
//		counter++;
//	}
//	// --------------------------------------------------
//	// 数えた文字数だけ、line_buf_p に文字列をコピー
//	// --------------------------------------------------
//	memset(line_buf_p, '\0', line_buf_size);
//	if (counter >= line_buf_size) {
//		counter = (line_buf_size - 1);
//	}
//	strncpy(line_buf_p, text_buf_p, counter);
//	if (*p == '\0') {
//		return NULL;            // バッファの最後
//	}
//	else {
//		return p;               // バッファの途中
//	}
//}
// **************************************************************************
//  URIエンコードを行います.
//  機能 : URIデコードを行う
//  書式 : int uri_encode
//  (char* dst,size_t dst_len,const char* src,int src_len);
//  引数 : dst 変換した文字の書き出し先.
//                 dst_len 変換した文字の書き出し先の最大長.
//                 src 変換元の文字.
//                 src_len 変換元の文字の長さ.
//  返値 : エンコードした文字の数(そのままも含む)
// **************************************************************************
//int uri_encode(char* dst, unsigned int dst_len, const char* src, unsigned int src_len)
//{
//	unsigned int idx_src;
//	unsigned int idx_dst;
//	int cnt;
//	// 引数チェック
//	if ((dst == NULL) || (dst_len < 1) || (src == NULL) || (src_len < 1)) {
//		return 0;
//	}
//	cnt = 0;
//	for (idx_src = idx_dst = 0; (idx_src < src_len) && (idx_dst < dst_len) && (src[idx_src] != '\0'); idx_src++) {
//		/* ' '(space) はちと特別扱いにしないとまずい */
//		if (src[idx_src] == ' ') {
//			//dst[idx_dst++] = '+';
//			dst[idx_dst++] = '%';
//			dst[idx_dst++] = '2';
//			dst[idx_dst++] = '0';
//		}
//		/* エンコードしない文字全員集合 */
//		else if (strchr("!$()*,-./:;?@[]^_`{}~", src[idx_src]) != NULL) {
//			dst[idx_dst] = src[idx_src];
//			idx_dst += 1;
//		}
//		/* アルファベットと数字はエンコードせずそのまま */
//		else if (isalnum(src[idx_src])) {
//			dst[idx_dst] = src[idx_src];
//			idx_dst += 1;
//		}
//		/* \マークはエンコード */
//		else if (strchr(DELIMITER, src[idx_src]) != NULL) {
//			dst[idx_dst++] = '%';
//			dst[idx_dst++] = '5';
//			dst[idx_dst++] = 'C';
//		}
//		/* それ以外はすべてエンコード */
//		else {
//			if ((idx_dst + 3) > dst_len)
//				break;
//			idx_dst += sprintf(&dst[idx_dst], "%%%2X", (unsigned char)(src[idx_src]));
//		}
//		cnt++;
//		if ((idx_dst + 1) < dst_len) {
//			dst[idx_dst] = '\0';
//		}
//	}
//	return cnt;
//	// 2004/10/01 Update end
//}
// **************************************************************************
// URIデコードを行います.
//  機能 : URIデコードを行う
//  引数 : dst 変換した文字の書き出し先.
//                dst_len 変換した文字の書き出し先の最大長.
//                src 変換元の文字.
//                src_len 変換元の文字の長さ.
// 返値 : デコードした文字の数(そのままも含む)
// **************************************************************************
int uri_decode(char* dst, unsigned int dst_len, const char* src, unsigned int src_len)
{
	unsigned int    idx_src;
	unsigned int    idx_dst;
	int             cnt;
	char            work[3] = {};
	//const char    *ptr_stop;
	char* strtol_end_ptr;
	int             code;
	// 引数チェック
	if ((dst == NULL) || (dst_len < 1) || (src == NULL) || (src_len < 1)) {
		return 0;
	}
	cnt = 0;
	// =================
	// メインループ
	// =================
	for (idx_src = idx_dst = 0; (idx_src < src_len) && (idx_dst < dst_len) && (src[idx_src] != '\0'); idx_dst++, cnt++) {
		if (src[idx_src] == '%') {
			if (idx_src + 2 > src_len) {
				break;
			}
			work[0] = src[idx_src + 1];
			work[1] = src[idx_src + 2];
			work[2] = '\0';
			code = strtol(work, &strtol_end_ptr, 16);
			//ptr_stop = &src[idx_src + (strtol_end_ptr - work) + 1];
			if (code == LONG_MIN || code == LONG_MAX) {
				break;
			}
			if (strtol_end_ptr != NULL) {
				if (*strtol_end_ptr != '\0') {
					break;
				}
			}
			dst[idx_dst] = (char)code;
			idx_src += 3;
		}
		else if (src[idx_src] == '+') {
			dst[idx_dst] = ' ';
			idx_src += 1;
			//ptr_stop++;
		}
		else {
			dst[idx_dst] = src[idx_src];
			idx_src += 1;
			//ptr_stop++;
		}
		if (idx_dst + 1 < dst_len) {
			dst[idx_dst + 1] = '\0';
		}
	}
	return cnt;
}


//---------------------------------------------------------------------------
char* path_sanitize(char* orig_dir, size_t dir_size)
{
	IGNORE_PARAMETER(dir_size);
	//wString temp;
	//temp = orig_dir;
	//char work[1024] = {};
	//nkf (orig_dir, work, dir_size * 3, "s");
	//strcpy (orig_dir, work);
	//

#ifdef linux
	char* p;
	char* q;
	char* dir;
	char* buf;
	size_t malloc_len;
	if (orig_dir == NULL) return NULL;
	malloc_len = strlen(orig_dir) * 2;
	buf = (char*)malloc(malloc_len);
	buf[0] = '\0';
	p = buf;
	dir = q = orig_dir;
	while (q != NULL) {
		dir = q;
		while (*dir == '/') dir++;
		q = strchr(dir, '/');
		if (q != NULL) {
			*q++ = '\0';
		}
		if (!strcmp(dir, "..")) {
			p = strrchr(buf, '/');
			if (p == NULL) {
				free(buf);
				dir[0] = '\0';
				return NULL; //  not allowed.
			}
			*p = '\0';
		}
		else if (strcmp(dir, ".")) {
			p += snprintf(p, malloc_len - (p - buf), "/%s", dir);
		}
	}
	if (buf[0] == '\0') {
		strncpy(orig_dir, "/", dir_size);
	}
	else {
		strncpy(orig_dir, buf, dir_size);
	}
	free(buf);
#endif
	return orig_dir;
}

wString GetAuthorization(const wString& AuthorizedString)
{
	wString auth;
	if (AuthorizedString.length()) {
		auth.sprintf("Authorization: Basic %s", AuthorizedString.c_str());
	}
	return auth;
}

#ifdef linux
//---------------------------------------------------------------------------
int send(int fd, const char* buffer, unsigned int length, int mode)
{
	return write(fd, buffer, length);
}
//---------------------------------------------------------------------------
int recv(int fd, char* buffer, unsigned int length, int mode)
{
	return read(fd, buffer, length);
}
//---------------------------------------------------------------------------
int getTargetFile(const char* LinkFile, char* TargetFile)
{
	return FALSE;
}
//---------------------------------------------------------------------------
void Sleep(unsigned int milliseconds)
{
	unsigned long sec = milliseconds / 1000;
	milliseconds %= 1000;
	//linuxでは秒タイマー
	if (sec) {
		sleep(sec);
	}
	//linuxではμ秒タイマー
	if (milliseconds) {
		usleep(milliseconds * 1000);
	}
}
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/// <summary>
/// linux/windows共用オープン
/// 追加: O_CREAT | O_APPEND | O_WRONLY(またはO_RDWR) | (O_BINARY) , S_IREAD | S_IWRITE
/// 新規: O_CREAT | O_TRUNC  | O_WRONLY(またはO_RDWR) | (O_BINARY) , S_IREAD | S_IWRITE
/// 読込: O_RDONLY   
/// </summary>
/// <param name="filename">オープンするファイル名</param>
/// <param name="amode">アクセスモード</param>
/// <param name="option"></param>
/// <returns></returns>
int myopen(const wString& filename, int amode, int option)
{
#ifdef linux
	if (option != 0) {
		return open(filename.c_str(), amode, option);
	}
	else {
		return open(filename.c_str(), amode);
	}
#else
	wString FileNamew = filename;
	if (option != 0) {
		auto ret =  open(FileNamew.c_str(), amode, option);
		return ret;
	}
	else {
		auto ret =  open(FileNamew.c_str(), amode);
		return ret;
	}
#endif
}


