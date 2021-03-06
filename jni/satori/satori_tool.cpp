#include	"satori.h"
#ifdef POSIX
#  include      "Utilities.h"
#else
#  include	<mbctype.h>	// for _ismbblead,_ismbbtrail
#endif


#include	<fstream>
#include	<cassert>

#ifdef POSIX
#  include <iostream>
#  include <climits>
#endif

#ifndef POSIX
// ファイルの最終更新日時を取得
bool	GetLastWriteTime(LPCSTR iFileName, SYSTEMTIME& oSystemTime) {
	HANDLE	theFile = ::CreateFile( iFileName, 
		GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( theFile==INVALID_HANDLE_VALUE )
		return	false;

	BY_HANDLE_FILE_INFORMATION	theInfo;
	::GetFileInformationByHandle(theFile, &theInfo);
	::CloseHandle(theFile);

	FILETIME	FileTime;
	::FileTimeToLocalFileTime(&(theInfo.ftLastWriteTime), &FileTime);
	::FileTimeToSystemTime(&FileTime, &oSystemTime);
	return	true;
}
#endif


//----------------------------------------------------------------------
//	ファイルの更新日時を比較。
//	返値が正ならば前者、負ならば後者のほうが新しいファイル。
//----------------------------------------------------------------------
#ifdef POSIX
#include <sys/types.h>
#include <sys/stat.h>
int CompareTime(const string& file1, const string& file2) {
    // file1の方が新しければ1、同じなら0、古ければ-1。
    struct stat s1, s2;
    int r1 = ::stat(file1.c_str(), &s1);
    int r2 = ::stat(file2.c_str(), &s2);
    if (r1 == 0) {
	if (r2 != 0) {
	    return 1;
	}
    }
    else {
	if (r2 == 0) {
	    return -1;
	}
	else {
	    return 0;
	}
    }
    if (s1.st_mtime > s2.st_mtime) {
	return 1;
    }
    else if (s1.st_mtime < s2.st_mtime) {
	return -1;
    }
    else {
	return 0;
    }
}
#else
int	CompareTime(LPCSTR szL, LPCSTR szR) {
	assert(szL!=NULL && szR!=NULL);

	SYSTEMTIME	stL, stR;
	BOOL		fexistL, fexistR;

	// 更新日付を得る。
	fexistL = GetLastWriteTime(szL, stL);
	fexistR	= GetLastWriteTime(szR, stR);
	// 存在しないファイルは「古い」と見なす。
	if ( fexistL ) {
		if ( !fexistR)
			return	1;
	} else {
		if ( fexistR )
			return	-1;
		else
			return	0;	// どっちもありゃしねぇ
	}

	// 最終更新日付を比較
	if ( stL.wYear > stR.wYear )	return	1;
	else if ( stL.wYear < stR.wYear )	return	-1;
	if ( stL.wMonth > stR.wMonth )	return	1;
	else if ( stL.wMonth < stR.wMonth )	return	-1;
	if ( stL.wDay > stR.wDay )	return	1;
	else if ( stL.wDay < stR.wDay )	return	-1;
	if ( stL.wHour > stR.wHour )	return	1;
	else if ( stL.wHour < stR.wHour )	return	-1;
	if ( stL.wMinute > stR.wMinute )	return	1;
	else if ( stL.wMinute < stR.wMinute )	return	-1;
	if ( stL.wSecond > stR.wSecond )	return	1;
	else if ( stL.wSecond < stR.wSecond )	return	-1;
	if ( stL.wMilliseconds > stR.wMilliseconds )	return	1;
	else if ( stL.wMilliseconds < stR.wMilliseconds )	return	-1;
	// 制作日時の完全一致
	return	0;
}
#endif


string	zen2han(string str) {
	static const char	before[] = "０１２３４５６７８９ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ−＋";
	static const char	after[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-+";
	char	buf1[3]="\0\0", buf2[2]="\0";
	for (int n=0 ; n<sizeof(after) ; ++n) {
		buf1[0]=before[n*2];
		buf1[1]=before[n*2+1];
		buf2[0]=after[n];
		replace(str, buf1, buf2);
	}
	return	str;
}

string int2zen(int i) {
	static const char*	ary[] = {"０","１","２","３","４","５","６","７","８","９"};

	string	zen;
	if ( i<0 ) {
		zen += "−";
		i = -i; // INT_MINの時は符号が反転しない
	}
	string	han=itos(i);
	const char* p=han.c_str();
	if ( i==INT_MIN )
		++p;
	for (  ; *p != '\0' ; ++p ) {
		assert(*p>='0' && *p<='9');
		zen += ary[*p-'0'];
	}
	return	zen;
}


string	Satori::GetWord(const string& name) {
	return "いぬ";
}

string	Satori::surface_restore_string() { 
	string	str="";
	if ( !surface_restore_at_talk )	// そもそも必要なし、の場合
		return	"\\1";

	//for ( set<int>::const_iterator i=surface_changed_before_speak.begin() ; i!=surface_changed_before_speak.end() ; ++i )

	for ( map<int, int>::const_iterator i=default_surface.begin() ; i!=default_surface.end() ; ++i ) {
		if ( mIsMateria ) {
			if ( i->first >= 2 )
				continue;
			else if ( surface_changed_before_speak.find(i->first) == surface_changed_before_speak.end() )
				str += string() + "\\" + itos(i->first) + "\\s[" + itos(i->second) + "]";
		} else
			if ( surface_changed_before_speak.find(i->first) == surface_changed_before_speak.end() )
				str += string() + "\\p[" + itos(i->first) + "]\\s[" + itos(i->second) + "]";
	}

	surface_changed_before_speak.clear();
	return	str;
}


// ある名前により指定される「全ての」URL及び付帯情報、のリスト
bool	Satori::GetURLList(const string& name, string& result)
{
	Family<Talk>* f = talks.get_family(name);
	if ( f == NULL )
		return false;

	vector<const Talk*> tg;
	f->get_elements_pointers(tg);
	for ( vector<const Talk*>::iterator it = tg.begin() ; it != tg.end() ; ++it )
	{
		const Talk& vec = **it;
		if ( vec.size() < 1 )
			continue;
		string	menu = vec[0];
		string	url = (vec.size()<2) ? ("") : (vec[1]);
		string	banner = (vec.size()<3) ? ("") : (vec[2]);
		int	len = menu.size()+1+url.size()+1+banner.size()+1;
		char*	buf=new char[len+1];
		sprintf(buf, "%s%c%s%c%s%c", menu.c_str(), 1, url.c_str(), 1, banner.c_str(), 2);
		result += buf;
		delete [] buf;
	}
	return	true;
}

// ある名前により指定されるURL中の指定サイトのスクリプトを取得
bool	Satori::GetRecommendsiteSentence(const string& name, string& result)
{
	Family<Talk>* f = talks.get_family(name);
	if ( f == NULL )
		return false;

	vector<const Talk*> tg;
	f->get_elements_pointers(tg);
	for ( vector<const Talk*>::iterator it = tg.begin() ; it != tg.end() ; ++it )
	{
		const Talk& t = **it;
		if ( t.size() >= 4 && t[0]==mReferences[0] )
		{
			result = SentenceToSakuraScript( Talk(t.begin()+3, t.end()) );
			return	true;
		}
	}
	return	false;
}

strmap*	Satori::find_ghost_info(string name) {
	vector<strmap>::iterator i=ghosts_info.begin();
	for ( ; i!=ghosts_info.end() ; ++i )
		if ( (*i)["name"] == name )
			return	&(*i);
	return	NULL;
}




// 文章の中で （ を見つけた場合、pが （ の次の位置まで進められた上でこれが実行される。
// pはこの内部で ） の次の位置まで進められる。
// 返値はカッコの解釈結果。
string	Satori::KakkoSection(const char*& p) {
	string	kakko_str;
	while (true) {
		if ( p[0] == '\0' )
			return	string("（") + kakko_str;	// 閉じカッコが無かった

		string c = get_a_chr(p);
		if ( c=="）" )
			break;
		else if ( c=="（" ) {
			kakko_str += KakkoSection(p);
		}
		else
			kakko_str += c;
	}

	string	result;
	if ( Call(kakko_str, result) )
		return	result;
	if ( unkakko_for_calcurate )
		return	string("０");
	else
		return	string("（") + kakko_str + "）";
}

string	Satori::UnKakko(const char* p) {
	assert(p!=NULL);
	string	result;
	while ( p[0] != '\0' ) {
		string c=get_a_chr(p);
		result += (c=="（") ? KakkoSection(p) : c;
	}
	return	result;
}

void	Satori::erase_var(const string& key) {
	if ( key == "スコープ切り換え時" )
		append_at_scope_change = "";
	else if ( key == "さくらスクリプトによるスコープ切り換え時" )
		append_at_scope_change_with_sakura_script = "";
	variables.erase(key);
}

bool	Satori::system_variable_operation(string key, string value, string* result)
{
	// mapにしようよ。

	if ( key == "喋り間隔" ) {
		talk_interval = stoi( zen2han(value) );
		if ( talk_interval<3 ) talk_interval=0; // 3未満は喋らない

		// 喋りカウント初期化
		int	dist = int(talk_interval*(talk_interval_random/100.0));
		talk_interval_count = ( dist==0 ) ? talk_interval : 
			(talk_interval-dist)+(random()%(dist*2));
	}
	else if ( key == "喋り間隔誤差" ) {
		talk_interval_random = stoi( zen2han(value) );
		if ( talk_interval_random>100 ) talk_interval_random=100;
		if ( talk_interval_random<0 ) talk_interval_random=0;

		// 喋りカウント初期化
		int	dist = int(talk_interval*(talk_interval_random/100.0));
		talk_interval_count = ( dist==0 ) ? talk_interval : 
			(talk_interval-dist)+(random()%(dist*2));
	}
	else if ( key == "スコープ切り換え時" ) {
		append_at_scope_change = zen2han(value);
	}
	else if ( key == "さくらスクリプトによるスコープ切り換え時" ) {
		append_at_scope_change_with_sakura_script = zen2han(value);
	}
	else if ( key == "トーク開始時" ) {
		append_at_talk_start = zen2han(value);
	}
	else if ( key == "トーク終了時" ) {
		append_at_talk_end = zen2han(value);
	}
	else if ( key == "会話時サーフェス戻し" ) {
		surface_restore_at_talk=(value=="有効");
	}
	else if ( compare_head(key,  "サーフェス加算値") && aredigits(key.c_str() + strlen("サーフェス加算値")) ) {
		int n = atoi(key.c_str() + strlen("サーフェス加算値"));
		surface_add_value[n]=stoi( zen2han(value) );

		variables[string()+"デフォルトサーフェス"+itos(n)] = value;
		next_default_surface[n]=stoi( zen2han(value) );
		if ( !is_speaked_anybody() )
			default_surface[n]=next_default_surface[n];
	}
	else if ( compare_head(key,  "デフォルトサーフェス") && aredigits(key.c_str() + strlen("デフォルトサーフェス")) ) {
		int n = atoi(key.c_str() + strlen("デフォルトサーフェス"));
		next_default_surface[n]=stoi( zen2han(value) );
		if ( !is_speaked_anybody() )
			default_surface[n]=next_default_surface[n];
	}
	else if ( compare_head(key,  "BalloonOffset") && aredigits(key.c_str() + strlen("BalloonOffset")) ) {
		int n = atoi(key.c_str() + strlen("BalloonOffset"));
		BalloonOffset[n] = value;
		validBalloonOffset[n] = true;
	}
	else if ( key == "トーク中のなでられ反応") {
		insert_nade_talk_at_other_talk= (value=="有効");
	}
	else if ( key == "なでられ持続秒数") {
		nade_valid_time_initializer = stoi( zen2han(value) );
	}
	else if ( key == "なでられ反応回数") {
		nade_sensitivity = stoi( zen2han(value) );
	}
	else if ( key == "Log" ) {
		Sender::validate(value=="有効");
	}
	else if ( key == "RequestLog" ) {
		fRequestLog = (value=="有効");
	}
	else if ( key == "OperationLog" ) {
		fOperationLog = (value=="有効");
	}
	else if ( key == "ResponseLog" ) {
		fResponseLog = (value=="有効");
	}
	else if ( key == "自動挿入ウェイトの倍率" ) {
		rate_of_auto_insert_wait=stoi( zen2han(value) );
		rate_of_auto_insert_wait = min(1000, max(0, rate_of_auto_insert_wait));
		variables["自動挿入ウェイトの倍率"] = itos(rate_of_auto_insert_wait);
	}
	else if ( key == "辞書フォルダ" ) {
		strvec	words;
		split(value, ",",dic_folder);
		reload_flag=true;
	}
	else if ( key == "セーブデータ暗号化" ) {
		fEncodeSavedata = (value=="有効");
	}
	else if ( compare_head(key,"単語群「") && compare_tail(key,"」の重複回避") ) {
		variables.erase(key);
		words.setOC( string(key.c_str()+8, key.length()-8-12), value );
	}
	else if ( compare_head(key,"文「") && compare_tail(key,"」の重複回避") ) {
		variables.erase(key);
		talks.setOC( string(key.c_str()+4, key.length()-4-12), value );
	}
	else if ( key == "次のトーク" ) {
		variables.erase(key);
		int	count=1;
		while ( reserved_talk.find(count) != reserved_talk.end() )
			++count;
		reserved_talk[count] = value;
		sender << "次回のランダムトークが「" << value << "」に予\x96\xf1されました。" << endl;
	}
	else if ( compare_head(key,"次から") && compare_tail(key,"回目のトーク") ) {
		variables.erase(key);
		int	count = stoi( zen2han( string(key.c_str()+6, key.length()-6-12) ) );
		if ( count<=0 ) {
			sender << "トーク予\x96\xf1、設定値がヘンです。" << endl;
		}
		else {
			while ( reserved_talk.find(count) != reserved_talk.end() )
				++count;
			reserved_talk[count] = value;
			sender << count << "回後のランダムトークが「" << value << "」に予\x96\xf1されました。" << endl;
		}
	}
	else if ( key=="トーク予\x96\xf1のキャンセル" ) {
		if ( value=="＊" )
			reserved_talk.clear();
		else
			for (map<int, string>::iterator it=reserved_talk.begin(); it!=reserved_talk.end() ; )
				if ( value == it->second )
					reserved_talk.erase(it++);
				else
					++it;
	}
	else if ( key == "SAORI引数の計算" ) {
		if (value=="有効")
			mSaoriArgumentCalcMode = SACM_ON;
		else if (value=="無効")
			mSaoriArgumentCalcMode = SACM_OFF;
		else
			mSaoriArgumentCalcMode = SACM_AUTO;
	}
	else if ( key == "辞書リロード" && value=="実行") {
		variables.erase(key);
		reload_flag=true;
	}
	else if ( key == "手動セーブ" && value=="実行") {
		variables.erase(key);
		this->Save();
	}
	else if ( key == "自動セーブ間隔" ) {
		mAutoSaveInterval = stoi(zen2han(value));
		mAutoSaveCurrentCount = mAutoSaveInterval;
		if ( mAutoSaveInterval > 0 )
			sender << ""  << itos(mAutoSaveInterval) << "秒間隔で自動セーブを行います。" << endl;
		else
			sender << "自動セーブは行いません。" << endl;
	}
	else if ( key == "全タイマ解除" && value=="実行") {
		variables.erase(key);
		for (strintmap::iterator i=timer.begin();i!=timer.end();++i)
			variables.erase(i->first + "タイマ");
		timer.clear();
	}
	else if ( key == "教わること" ) {
		variables.erase(key);
		teach_genre=value;
		if ( result != NULL )
			*result += "\\![open,teachbox]";
	}
	else if ( key.size()>6 && compare_tail(key, "タイマ") ) {
		string	name(key.c_str(), strlen(key.c_str())-6);
		/*if ( sentences.find(name) == sentences.end() ) {
			result = string("※　タイマ終了時のジャンプ先 ＊")+name+" がありません　※";
			// セーブデータ復帰時を考慮
		}
		else */{
			int sec = stoi(zen2han(value));
			if ( sec < 1 ) {
				variables.erase(key);
				if ( timer.find(name)!=timer.end() ) {
					timer.erase(name);
					sender << "タイマ「"  << name << "」の予\x96\xf1がキャンセルされました。" << endl;
				} else
					sender << "タイマ「"  << name << "」は元から予\x96\xf1されていません。" << endl;
			} else {
				timer[name] = sec;
				sender << "タイマ「"  << name << "」が" << sec << "秒後に予\x96\xf1されました。" << endl;
			}
		}
	}
	else if ( key == "引数区切り追加" && value.size()>0 ) {
		variables.erase(key);
		mDelimiters.insert(value);
	}
	else if ( key == "引数区切り削除" && value.size()>0 ) {
		variables.erase(key);
		mDelimiters.erase(value);
	}
	else if ( compare_head(key, "Value") && aredigits(key.substr(5)) )
	{
		variables.erase(key);
		mResponseMap[string()+"Reference"+key.substr(5)] = value;
	}
	else
		return	false;

	return	true;
}


bool	Satori::calculate(const string& iExpression, string& oResult) {

	bool	tmp = unkakko_for_calcurate;
	unkakko_for_calcurate = true;
	oResult = UnKakko(iExpression.c_str());
	unkakko_for_calcurate = tmp;

	bool r = calc(oResult);
	if ( !r ) {
#ifdef POSIX
	        std::cerr <<
		    "error on Satori::calculate" << std::endl <<
		    "Error in expression: " << iExpression << std::endl;
#else
		// もうちょっと抽象化を……
		::MessageBox(NULL, (string() + "式が計算不能です。\n" + iExpression).c_str(), "error on Satori::calculate" , MB_OK);
#endif
	}
	return	r;
}

