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
// �t�@�C���̍ŏI�X�V�������擾
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
//	�t�@�C���̍X�V�������r�B
//	�Ԓl�����Ȃ�ΑO�ҁA���Ȃ�Ό�҂̂ق����V�����t�@�C���B
//----------------------------------------------------------------------
#ifdef POSIX
#include <sys/types.h>
#include <sys/stat.h>
int CompareTime(const string& file1, const string& file2) {
    // file1�̕����V�������1�A�����Ȃ�0�A�Â����-1�B
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

	// �X�V���t�𓾂�B
	fexistL = GetLastWriteTime(szL, stL);
	fexistR	= GetLastWriteTime(szR, stR);
	// ���݂��Ȃ��t�@�C���́u�Â��v�ƌ��Ȃ��B
	if ( fexistL ) {
		if ( !fexistR)
			return	1;
	} else {
		if ( fexistR )
			return	-1;
		else
			return	0;	// �ǂ���������Ⴕ�˂�
	}

	// �ŏI�X�V���t���r
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
	// ��������̊��S��v
	return	0;
}
#endif


string	zen2han(string str) {
	static const char	before[] = "�O�P�Q�R�S�T�U�V�W�X�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�o�p�q�r�s�t�u�v�w�x�y�����������������������������������������������������|�{";
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
	static const char*	ary[] = {"�O","�P","�Q","�R","�S","�T","�U","�V","�W","�X"};

	string	zen;
	if ( i<0 ) {
		zen += "�|";
		i = -i; // INT_MIN�̎��͕��������]���Ȃ�
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
	return "����";
}

string	Satori::surface_restore_string() { 
	string	str="";
	if ( !surface_restore_at_talk )	// ���������K�v�Ȃ��A�̏ꍇ
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


// ���閼�O�ɂ��w�肳���u�S�ẮvURL�y�ѕt�я��A�̃��X�g
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

// ���閼�O�ɂ��w�肳���URL���̎w��T�C�g�̃X�N���v�g���擾
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




// ���͂̒��� �i ���������ꍇ�Ap�� �i �̎��̈ʒu�܂Ői�߂�ꂽ��ł��ꂪ���s�����B
// p�͂��̓����� �j �̎��̈ʒu�܂Ői�߂���B
// �Ԓl�̓J�b�R�̉��ߌ��ʁB
string	Satori::KakkoSection(const char*& p) {
	string	kakko_str;
	while (true) {
		if ( p[0] == '\0' )
			return	string("�i") + kakko_str;	// ���J�b�R����������

		string c = get_a_chr(p);
		if ( c=="�j" )
			break;
		else if ( c=="�i" ) {
			kakko_str += KakkoSection(p);
		}
		else
			kakko_str += c;
	}

	string	result;
	if ( Call(kakko_str, result) )
		return	result;
	if ( unkakko_for_calcurate )
		return	string("�O");
	else
		return	string("�i") + kakko_str + "�j";
}

string	Satori::UnKakko(const char* p) {
	assert(p!=NULL);
	string	result;
	while ( p[0] != '\0' ) {
		string c=get_a_chr(p);
		result += (c=="�i") ? KakkoSection(p) : c;
	}
	return	result;
}

void	Satori::erase_var(const string& key) {
	if ( key == "�X�R�[�v�؂芷����" )
		append_at_scope_change = "";
	else if ( key == "������X�N���v�g�ɂ��X�R�[�v�؂芷����" )
		append_at_scope_change_with_sakura_script = "";
	variables.erase(key);
}

bool	Satori::system_variable_operation(string key, string value, string* result)
{
	// map�ɂ��悤��B

	if ( key == "����Ԋu" ) {
		talk_interval = stoi( zen2han(value) );
		if ( talk_interval<3 ) talk_interval=0; // 3�����͒���Ȃ�

		// ����J�E���g������
		int	dist = int(talk_interval*(talk_interval_random/100.0));
		talk_interval_count = ( dist==0 ) ? talk_interval : 
			(talk_interval-dist)+(random()%(dist*2));
	}
	else if ( key == "����Ԋu�덷" ) {
		talk_interval_random = stoi( zen2han(value) );
		if ( talk_interval_random>100 ) talk_interval_random=100;
		if ( talk_interval_random<0 ) talk_interval_random=0;

		// ����J�E���g������
		int	dist = int(talk_interval*(talk_interval_random/100.0));
		talk_interval_count = ( dist==0 ) ? talk_interval : 
			(talk_interval-dist)+(random()%(dist*2));
	}
	else if ( key == "�X�R�[�v�؂芷����" ) {
		append_at_scope_change = zen2han(value);
	}
	else if ( key == "������X�N���v�g�ɂ��X�R�[�v�؂芷����" ) {
		append_at_scope_change_with_sakura_script = zen2han(value);
	}
	else if ( key == "�g�[�N�J�n��" ) {
		append_at_talk_start = zen2han(value);
	}
	else if ( key == "�g�[�N�I����" ) {
		append_at_talk_end = zen2han(value);
	}
	else if ( key == "��b���T�[�t�F�X�߂�" ) {
		surface_restore_at_talk=(value=="�L��");
	}
	else if ( compare_head(key,  "�T�[�t�F�X���Z�l") && aredigits(key.c_str() + strlen("�T�[�t�F�X���Z�l")) ) {
		int n = atoi(key.c_str() + strlen("�T�[�t�F�X���Z�l"));
		surface_add_value[n]=stoi( zen2han(value) );

		variables[string()+"�f�t�H���g�T�[�t�F�X"+itos(n)] = value;
		next_default_surface[n]=stoi( zen2han(value) );
		if ( !is_speaked_anybody() )
			default_surface[n]=next_default_surface[n];
	}
	else if ( compare_head(key,  "�f�t�H���g�T�[�t�F�X") && aredigits(key.c_str() + strlen("�f�t�H���g�T�[�t�F�X")) ) {
		int n = atoi(key.c_str() + strlen("�f�t�H���g�T�[�t�F�X"));
		next_default_surface[n]=stoi( zen2han(value) );
		if ( !is_speaked_anybody() )
			default_surface[n]=next_default_surface[n];
	}
	else if ( compare_head(key,  "BalloonOffset") && aredigits(key.c_str() + strlen("BalloonOffset")) ) {
		int n = atoi(key.c_str() + strlen("BalloonOffset"));
		BalloonOffset[n] = value;
		validBalloonOffset[n] = true;
	}
	else if ( key == "�g�[�N���̂Ȃł�ꔽ��") {
		insert_nade_talk_at_other_talk= (value=="�L��");
	}
	else if ( key == "�Ȃł�ꎝ���b��") {
		nade_valid_time_initializer = stoi( zen2han(value) );
	}
	else if ( key == "�Ȃł�ꔽ����") {
		nade_sensitivity = stoi( zen2han(value) );
	}
	else if ( key == "Log" ) {
		Sender::validate(value=="�L��");
	}
	else if ( key == "RequestLog" ) {
		fRequestLog = (value=="�L��");
	}
	else if ( key == "OperationLog" ) {
		fOperationLog = (value=="�L��");
	}
	else if ( key == "ResponseLog" ) {
		fResponseLog = (value=="�L��");
	}
	else if ( key == "�����}���E�F�C�g�̔{��" ) {
		rate_of_auto_insert_wait=stoi( zen2han(value) );
		rate_of_auto_insert_wait = min(1000, max(0, rate_of_auto_insert_wait));
		variables["�����}���E�F�C�g�̔{��"] = itos(rate_of_auto_insert_wait);
	}
	else if ( key == "�����t�H���_" ) {
		strvec	words;
		split(value, ",",dic_folder);
		reload_flag=true;
	}
	else if ( key == "�Z�[�u�f�[�^�Í���" ) {
		fEncodeSavedata = (value=="�L��");
	}
	else if ( compare_head(key,"�P��Q�u") && compare_tail(key,"�v�̏d�����") ) {
		variables.erase(key);
		words.setOC( string(key.c_str()+8, key.length()-8-12), value );
	}
	else if ( compare_head(key,"���u") && compare_tail(key,"�v�̏d�����") ) {
		variables.erase(key);
		talks.setOC( string(key.c_str()+4, key.length()-4-12), value );
	}
	else if ( key == "���̃g�[�N" ) {
		variables.erase(key);
		int	count=1;
		while ( reserved_talk.find(count) != reserved_talk.end() )
			++count;
		reserved_talk[count] = value;
		sender << "����̃����_���g�[�N���u" << value << "�v�ɗ\\x96\xf1����܂����B" << endl;
	}
	else if ( compare_head(key,"������") && compare_tail(key,"��ڂ̃g�[�N") ) {
		variables.erase(key);
		int	count = stoi( zen2han( string(key.c_str()+6, key.length()-6-12) ) );
		if ( count<=0 ) {
			sender << "�g�[�N�\\x96\xf1�A�ݒ�l���w���ł��B" << endl;
		}
		else {
			while ( reserved_talk.find(count) != reserved_talk.end() )
				++count;
			reserved_talk[count] = value;
			sender << count << "���̃����_���g�[�N���u" << value << "�v�ɗ\\x96\xf1����܂����B" << endl;
		}
	}
	else if ( key=="�g�[�N�\\x96\xf1�̃L�����Z��" ) {
		if ( value=="��" )
			reserved_talk.clear();
		else
			for (map<int, string>::iterator it=reserved_talk.begin(); it!=reserved_talk.end() ; )
				if ( value == it->second )
					reserved_talk.erase(it++);
				else
					++it;
	}
	else if ( key == "SAORI�����̌v�Z" ) {
		if (value=="�L��")
			mSaoriArgumentCalcMode = SACM_ON;
		else if (value=="����")
			mSaoriArgumentCalcMode = SACM_OFF;
		else
			mSaoriArgumentCalcMode = SACM_AUTO;
	}
	else if ( key == "���������[�h" && value=="���s") {
		variables.erase(key);
		reload_flag=true;
	}
	else if ( key == "�蓮�Z�[�u" && value=="���s") {
		variables.erase(key);
		this->Save();
	}
	else if ( key == "�����Z�[�u�Ԋu" ) {
		mAutoSaveInterval = stoi(zen2han(value));
		mAutoSaveCurrentCount = mAutoSaveInterval;
		if ( mAutoSaveInterval > 0 )
			sender << ""  << itos(mAutoSaveInterval) << "�b�Ԋu�Ŏ����Z�[�u���s���܂��B" << endl;
		else
			sender << "�����Z�[�u�͍s���܂���B" << endl;
	}
	else if ( key == "�S�^�C�}����" && value=="���s") {
		variables.erase(key);
		for (strintmap::iterator i=timer.begin();i!=timer.end();++i)
			variables.erase(i->first + "�^�C�}");
		timer.clear();
	}
	else if ( key == "����邱��" ) {
		variables.erase(key);
		teach_genre=value;
		if ( result != NULL )
			*result += "\\![open,teachbox]";
	}
	else if ( key.size()>6 && compare_tail(key, "�^�C�}") ) {
		string	name(key.c_str(), strlen(key.c_str())-6);
		/*if ( sentences.find(name) == sentences.end() ) {
			result = string("���@�^�C�}�I�����̃W�����v�� ��")+name+" ������܂���@��";
			// �Z�[�u�f�[�^���A�����l��
		}
		else */{
			int sec = stoi(zen2han(value));
			if ( sec < 1 ) {
				variables.erase(key);
				if ( timer.find(name)!=timer.end() ) {
					timer.erase(name);
					sender << "�^�C�}�u"  << name << "�v�̗\\x96\xf1���L�����Z������܂����B" << endl;
				} else
					sender << "�^�C�}�u"  << name << "�v�͌�����\\x96\xf1����Ă��܂���B" << endl;
			} else {
				timer[name] = sec;
				sender << "�^�C�}�u"  << name << "�v��" << sec << "�b��ɗ\\x96\xf1����܂����B" << endl;
			}
		}
	}
	else if ( key == "������؂�ǉ�" && value.size()>0 ) {
		variables.erase(key);
		mDelimiters.insert(value);
	}
	else if ( key == "������؂�폜" && value.size()>0 ) {
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
		// ����������ƒ��ۉ����c�c
		::MessageBox(NULL, (string() + "�����v�Z�s�\�ł��B\n" + iExpression).c_str(), "error on Satori::calculate" , MB_OK);
#endif
	}
	return	r;
}
