#include	"satori.h"
#include	"../_/Utilities.h"
#ifdef POSIX
#  include      "posix_utils.h"
#endif
#include	<time.h>


void	add_characters(const char* p, int& characters) {
	// ������X�N���v�g�Ƃ���ȊO�𕪊����ď�����������
	while (*p) {
		if (*p=='\\'||*p=='%') {
			++p;
			if (*p=='\\'||*p=='%')	// �G�X�P�[�v���ꂽ\, %
				continue;
			while (!_ismbblead(*p) && (isalpha(*p)||isdigit(*p)||*p=='!'||*p=='*'||*p=='&'||*p=='?'||*p=='_'))
				++p;
			if (*p=='[') {
				for (++p ; *p!=']' ;)
					if (p[0]=='\\' && p[1]==']')	// �G�X�P�[�v���ꂽ]
						++p;
					else
						p += _ismbblead(*p) ? 2 : 1;
			}
		}
		else {
			int len = _ismbblead(*p) ? 2 : 1;
			p += len;
			characters += len;
		}
	}
}

#ifndef POSIX
static	SYSTEMTIME	DwordToSystemTime(DWORD dw) {
	SYSTEMTIME	st = { 0, 0, 0, 0, 0, 0, 0, 0 };
	st.wMilliseconds=WORD(dw%1000); dw/=1000;
	st.wSecond=WORD(dw%60); dw/=60;
	st.wMinute=WORD(dw%60); dw/=60;
	st.wHour=WORD(dw);
	return	st;
}
#endif

string	Satori::inc_call(
	const string& iCallName, 
	const strvec& iArgv, 
	strvec& oResults, 
	bool iIsSecure) 
{
	if ( !iIsSecure ) {
		sender << "local/Local�łȂ��̂ŏR��܂���: " << iCallName << endl;
		return	"";
	}

	if ( iCallName=="set" ) {
		if ( iArgv.size()==2 ) {
			string	result, key=iArgv[0], value=iArgv[1];

			if ( aredigits(zen2han(key)) ) {
				sender << "��" << key << "�@�����݂̂̕ϐ����͈����܂���." << endl;
				erase_var(key);	// ���ݖ���
			}
			else if ( value=="" ) {
				sender << "��" << key << "�^cleared." << endl;
				erase_var(key);	// ���ݖ���
			}
			else {
				sender << "��" << key << "��" << value << "�^" << 
					(( variables.find(key) == variables.end() ) ?
					"writed." : "overwrited.")<< endl;

				variables[key] = value;
				system_variable_operation(key, value, &result);
			}
			return	result;
		}
	}
	else if ( iCallName=="loop" ) {
		int	init=1, max=0, step=1, arg_size=iArgv.size();
		if ( arg_size==2 )
			max=stoi(iArgv[1]);
		else if ( arg_size==3 ) {
			init=stoi(iArgv[1]);
			max=stoi(iArgv[2]);
		}
		else if ( arg_size==4 ) {
			init=stoi(iArgv[1]);
			max=stoi(iArgv[2]);
			step=stoi(iArgv[3]);
		}
		else
			return	"";
		string	name=iArgv[0];
		string	ret,temp;

		if ( step==0 )
			return	"";
		else if ( step>0 ) {
			if ( init>max )
				return	"";
			for (int i=init ; i<=max ; i+=step ) {
				variables[name+"�J�E���^"] = itos(i);
				if ( !Call(name, temp) )
					return	"";
				ret += temp;
			}
		}
		else {
			if ( init<max )
				return	"";
			for (int i=init ; i>=max ; i+=step ) {
				variables[name+"�J�E���^"] = itos(i);
				if ( !Call(name, temp) )
					return	"";
				ret += temp;
			}
		}
		variables.erase(name+"�J�E���^");
		return	ret;
	}
	else if ( iCallName=="sync" ) {
		string	str = "\\![raise,OnDirectSaoriCall";
		if ( !iArgv.empty() ) {
			string	arg;
			combine(arg, iArgv, ",");
			str += ",";
			str += arg;
		}
		str += "]";
		return	str;
	}
	else if ( iCallName=="remember" ) {
		if ( iArgv.size() == 1 ) {
			int	n = stoi(iArgv[0]);
			if ( mResponseHistory.size() > n )
				return	mResponseHistory[n];
		}
	}
	else if ( iCallName=="call" ) {
		if ( iArgv.size() >= 1 ) {
			mCallStack.push( strvec() );
			strvec&	v = mCallStack.top();
			for ( int i=1 ; i<iArgv.size() ; ++i )
				v.push_back( iArgv[i] );
			string	r = Call(iArgv[0]);
			mCallStack.pop();
			return	r;
		}
	}
	else if ( iCallName=="freeze" ) {


	}
	else if ( iCallName == "�P��̒ǉ�" ) {

		if ( iArgv.size() == 2 )
		{
			Family<Word>* f = words.get_family(iArgv[0]);
			if ( f == NULL || false == f->is_exist_element(iArgv[1]) )
			{
				mAppendedWords[ iArgv[0] ].push_back( f->add_element(iArgv[1]) );
				sender << "�P��Q�u" << iArgv[0] << "�v�ɒP��u" << iArgv[1] << "�v���ǉ�����܂����B" << endl;
			}
			else
			{
				sender << "�P��Q�u" << iArgv[0] << "�v�ɒP��u" << iArgv[1] << "�v�͊��ɑ��݂��܂��B" << endl;
			}
		}
		else
			sender << "error: '�P��̒ǉ�' : �������s���ł��B" << endl;

	}
	else if ( iCallName=="nop" ) {
	}
	return	"";
}


// �����ɓn���ꂽ���̂������̖��O�ł���Ƃ��A�u�������Ώۂ�����Βu��������B
bool	Satori::Call(const string& iName, string& oResult) {
	string	hankaku;
	strvec*	p_kakko_replace_history = kakko_replace_history.empty() ? NULL : &(kakko_replace_history.top());

	bool	_pre_called_=false;

	// SAORI�Ή�, �����֐��Ăяo����������
	{
		string	thePluginName="";
		set<string>::const_iterator theDelimiter = mDelimiters.end();

		const char* p = NULL;
		enum { NO_CALL, SAORI_CALL, INC_CALL } state = NO_CALL;

		if ( mShioriPlugins.find(iName) ) {
			thePluginName=iName;
			state = SAORI_CALL;
		} else {

			static set<string> inner_commands;
			if ( inner_commands.empty() ) {
				// �{����map<name, function>���ȁ[�@�ށ[
				inner_commands.insert("set");
				inner_commands.insert("nop");
				inner_commands.insert("sync");
				inner_commands.insert("loop");
				inner_commands.insert("remember");
				inner_commands.insert("�P��̒ǉ�");
				inner_commands.insert("call");
				inner_commands.insert("freeze");
				//inner_commands.insert("�P��̍폜");
				//inner_commands.insert("�P��̑���");
			}

			for (set<string>::const_iterator i=mDelimiters.begin() ; i!=mDelimiters.end() ; ++i) {
				p = strstr_hz(iName.c_str(), i->c_str());
				if ( p==NULL )
					continue;
				string	str(iName.c_str(), p-iName.c_str());
				if ( mShioriPlugins.find(str) ) {	// ���݊m�F
					thePluginName=str;
					theDelimiter=i;
					state = SAORI_CALL;
					break;
				}
				else if ( inner_commands.find(str)!=inner_commands.end() ) {
					thePluginName=str;
					theDelimiter=i;
					state = INC_CALL;
					break;
				}
			}
		}

		if ( state==NO_CALL ) {
			_pre_called_=false;
		}
		else
		{
			_pre_called_=true;
			strvec	theArguments, theResults;

			if ( p!=NULL )// ����������Ȃ�
			{
				assert(theDelimiter != mDelimiters.end());
				string argstr = UnKakko(p);

				while (true)
				{
					p += theDelimiter->size();
					const char* pdlmt = strstr_hz(p, theDelimiter->c_str());
					if ( pdlmt==NULL ) {
						theArguments.push_back(p);
						break;
					}
					theArguments.push_back( string(p,pdlmt-p) );
					p = pdlmt;
				}

				if ( mSaoriArgumentCalcMode!=SACM_OFF ) {
					for ( strvec::iterator i=theArguments.begin() ; i!=theArguments.end() ; ++i ) {
						if ( i->size()==0 )
							continue;
						if ( mSaoriArgumentCalcMode==SACM_AUTO ) {
							int	c = zen2han(*i).at(0);
							if ( c!='+' && c!='-' && !(c>='0' && c<='9') )
								continue;
						}

						bool calc(string&);	// declare
						string	exp = *i;
						if ( calc(exp) )
							*i=exp;
					}
				}
			}

			// �����n���ĕԒl���擾�A�ƁB
			if ( state==SAORI_CALL )
				oResult = mShioriPlugins.request(thePluginName, theArguments, theResults, secure_flag ? "Local" : "External" );
			else
				oResult = inc_call(thePluginName, theArguments, theResults, secure_flag);
			oResult = UnKakko(oResult.c_str());	// �Ԓl���ēx�J�b�R�W�J
			
			// �����Ԓl��ϐ��ɃZ�b�g
			int	id=0;
			for ( strvec::iterator i=theResults.begin() ; i!=theResults.end() ; ++i ) {
				if ( i->size()==0 )
					variables.erase( string("�r")+int2zen(id++));
				else
					variables[string("�r")+int2zen(id++)] = *i;
			}
		}
	}

	const Word* w;

	if ( _pre_called_ ) {
		// �O�i�K�ł��łɑΉ��J�b�R�W�J�ς�
	}
	else if ( (w = words.select(iName, *this)) != NULL )
	{
		// �P���I������
		sender << "��" << iName << endl;
		oResult = UnKakko( w->c_str() );
		speaked_speaker.insert(speaker);
		add_characters(oResult.c_str(), characters);
	}
	else if ( talks.is_exist(iName) ) {
		// ���ɒ�`������Ε����擾
		oResult = GetSentence(iName);
	}
	else if ( variables.find(iName) != variables.end() ) {
		// �ϐ����ł���Εϐ��̓��e��Ԃ�
		oResult = variables[iName];
	}
	else if ( aredigits(hankaku=zen2han(iName)) || (hankaku[0]=='-' && aredigits(hankaku.c_str()+1)) ) {
		// �T�[�t�F�X�؂�ւ�
		int	s = stoi(hankaku);
		if ( s != -1 ) // -1�́u�����v�Ȃ̂œ��ʈ���
			s += surface_add_value[speaker];
		oResult = string("\\s[") + itos(s) + "]";
		if ( !is_speaked(speaker) )
			surface_changed_before_speak.insert(speaker);
	}
	else if ( hankaku[0]=='R' && aredigits(hankaku.c_str()+1) ) {
		// Event�ʒm���̈����擾
		int	ref=atoi(hankaku.c_str()+1);
		oResult = (ref>=0 && ref<mReferences.size()) ? mReferences[ref] : "";
		//oResult = mRequestMap[ string("Reference") + (hankaku.c_str()+1) ];
	}
	else if ( hankaku[0]=='H' && p_kakko_replace_history!=NULL && aredigits(hankaku.c_str()+1) ) {
		// �ߋ��̒u�������������Q��
		int	num = atoi(hankaku.c_str() +1) - 1;
		if ( num>=0 && num < p_kakko_replace_history->size() )
			oResult = p_kakko_replace_history->at(num);
	}
	else if ( hankaku[0]=='A' && mCallStack.size()>0 && aredigits(hankaku.c_str()+1)) {
		// call�ɂ��Ăяo���̈������Q��
		int	num = atoi(hankaku.c_str() +1);
		strvec&	v = mCallStack.top();
		if ( num < v.size() )
			oResult = v.at(num);
	}
	else if ( hankaku=="argc" ) {
		// call�ɂ��Ăяo���̈������܂Ƃ߂�
		int	num = atoi(hankaku.c_str() +1) - 1;
		strvec&	v = mCallStack.top();
		if ( num < v.size() )
			oResult = v.at(num);
	}
	else if ( hankaku=="argv" ) {
		// call�ɂ��Ăяo���̈������܂Ƃ߂�
		int	num = atoi(hankaku.c_str() +1) - 1;
		strvec&	v = mCallStack.top();
		if ( num < v.size() )
			oResult = v.at(num);
	}
	else if ( strncmp(iName.c_str(), "����", 4)==0 && iName.size()>6 ) { 
		strvec	vec;
		if ( split( iName.c_str()+4, "�`", vec ) != 2 ) {
			oResult = "���@�����̎w�肪�ςł��@��";
		}
		else {
			int	bottom = stoi(zen2han(vec[0]));
			int	top = stoi(zen2han(vec[1]));
			if ( bottom > top )
				Swap(&bottom, &top);

			if ( bottom == top )
				oResult = int2zen(top);
			else 
				oResult = int2zen( ((unsigned)random())%(top-bottom+1) + bottom );
		}
	}
	else if ( iName == "���ݔN" ) {
#ifdef POSIX
	        time_t st = time(NULL);
	        oResult = int2zen(localtime(&st)->tm_year + 1900);
#else
		SYSTEMTIME st; ::GetLocalTime(&st); oResult=int2zen(st.wYear);
#endif
	}
	else if ( iName == "���ݗj��" ) {
#ifdef POSIX
	        time_t st = time(NULL);
		struct tm* st_tm = localtime(&st);
		static const char* const ary[7]={"��","��","��","��","��","��","�y"};
		oResult = (st_tm->tm_wday >= 0 && st_tm->tm_wday < 7) ? ary[st_tm->tm_wday] : "�H";
#else
		SYSTEMTIME st; ::GetLocalTime(&st);
		static const char* const ary[7]={"��","��","��","��","��","��","�y"};
		oResult = ( st.wDayOfWeek >= 0 && st.wDayOfWeek < 7 ) ? ary[st.wDayOfWeek] : "�H";
#endif
	}
#ifdef POSIX
	else if ( iName == "���݌�" ) { time_t st = time(NULL); oResult = int2zen(localtime(&st)->tm_mon + 1); }
	else if ( iName == "���ݓ�" ) { time_t st = time(NULL); oResult = int2zen(localtime(&st)->tm_mday); }
	else if ( iName == "���ݎ�" ) { time_t st = time(NULL); oResult = int2zen(localtime(&st)->tm_hour); }
	else if ( iName == "���ݕ�" ) { time_t st = time(NULL); oResult = int2zen(localtime(&st)->tm_min); }
	else if ( iName == "���ݕb" ) { time_t st = time(NULL); oResult = int2zen(localtime(&st)->tm_sec); }
#else
	else if ( iName == "���݌�" ) { SYSTEMTIME st; ::GetLocalTime(&st); oResult=int2zen(st.wMonth); }
	else if ( iName == "���ݓ�" ) { SYSTEMTIME st; ::GetLocalTime(&st); oResult=int2zen(st.wDay); }
	else if ( iName == "���ݎ�" ) { SYSTEMTIME st; ::GetLocalTime(&st); oResult=int2zen(st.wHour); }
	else if ( iName == "���ݕ�" ) { SYSTEMTIME st; ::GetLocalTime(&st); oResult=int2zen(st.wMinute); }
	else if ( iName == "���ݕb" ) { SYSTEMTIME st; ::GetLocalTime(&st); oResult=int2zen(st.wSecond); }
#endif
#ifdef POSIX
	else if (iName == "�N����") {
	    unsigned long msec = posix_get_current_millis() - tick_count_at_load;
	    int hour = msec / 1000 / 60 / 60;
	    oResult = int2zen(hour);
	}
	else if (iName == "�N����") {
	    unsigned long msec = posix_get_current_millis() - tick_count_at_load;
	    int hour = msec / 1000 / 60 / 60;
	    msec -= hour * 60 * 60 * 1000;
	    int minute = msec / 1000 / 60;
	    oResult = int2zen(minute);
	}
	else if (iName == "�N���b" ) {
	    unsigned long msec = posix_get_current_millis() - tick_count_at_load;
	    int hour = msec / 1000 / 60 / 60;
	    msec -= hour * 60 * 60 * 1000;
	    int minute = msec / 1000 / 60;
	    msec -= minute * 60 * 1000;
	    int second = msec / 1000;
	    oResult = int2zen(second);
	}
	else if (iName == "�P���N���b" ) {
	    unsigned long msec = posix_get_current_millis() - tick_count_at_load;
	    oResult = int2zen(msec / 1000);
	}
	else if (iName == "�P���N����") {
	    unsigned long msec = posix_get_current_millis() - tick_count_at_load;
	    oResult = int2zen(msec / 1000 / 60);
	}
#else
	else if ( iName == "�N����" ) { oResult=int2zen(DwordToSystemTime(::GetTickCount()-tick_count_at_load).wHour); }
	else if ( iName == "�N����" ) { oResult=int2zen(DwordToSystemTime(::GetTickCount()-tick_count_at_load).wMinute); }
	else if ( iName == "�N���b" ) { oResult=int2zen(DwordToSystemTime(::GetTickCount()-tick_count_at_load).wSecond); }
	else if ( iName == "�P���N���b" ) { oResult=int2zen( (::GetTickCount()-tick_count_at_load)/1000 ); }
	else if ( iName == "�P���N����" ) { oResult=int2zen( (::GetTickCount()-tick_count_at_load)/1000/60 ); }
#endif
#ifdef POSIX
	else if (iName == "�n�r�N����" || iName == "�n�r�N����" || iName == "�n�r�N���b" ||
		 iName == "�P���n�r�N���b" || iName == "�P���n�r�N����") {
	    // �擾������@�������B
	    oResult = int2zen(0);
	}
#else
	else if ( iName == "�n�r�N����" ) { oResult=int2zen(DwordToSystemTime(::GetTickCount()).wHour); }
	else if ( iName == "�n�r�N����" ) { oResult=int2zen(DwordToSystemTime(::GetTickCount()).wMinute); }
	else if ( iName == "�n�r�N���b" ) { oResult=int2zen(DwordToSystemTime(::GetTickCount()).wSecond); }
	else if ( iName == "�P���n�r�N���b" ) { oResult=int2zen( ::GetTickCount() / 1000 ); }
	else if ( iName == "�P���n�r�N����" ) { oResult=int2zen( ::GetTickCount() / 1000/60 ); }
#endif
#ifdef POSIX
	else if (iName == "�݌v��") {
	    unsigned long msec = posix_get_current_millis() - tick_count_at_load + tick_count_total;
	    int hour = msec / 1000 / 60 / 60;
	    oResult = int2zen(hour);
	}
	else if (iName == "�݌v��" ) {
	    unsigned long msec = posix_get_current_millis() - tick_count_at_load + tick_count_total;
	    int hour = msec / 1000 / 60 / 60;
	    msec -= hour * 60 * 60 * 1000;
	    int minute = msec / 1000 / 60;
	    oResult = int2zen(minute);
	}
	else if (iName == "�݌v�b") {
	    unsigned long msec = posix_get_current_millis() - tick_count_at_load + tick_count_total;
	    int hour = msec / 1000 / 60 / 60;
	    msec -= hour * 60 * 60 * 1000;
	    int minute = msec / 1000 / 60;
	    msec -= minute * 60 * 1000;
	    int second = msec / 1000;
	    oResult = int2zen(second);
	}
	else if (iName == "�P���݌v�b") {
	    unsigned long msec = posix_get_current_millis() - tick_count_at_load + tick_count_total;
	    oResult = int2zen(msec / 1000);
	}
	else if (iName == "�P���݌v��") {
	    unsigned long msec = posix_get_current_millis() - tick_count_at_load + tick_count_total;
	    oResult = int2zen(msec / 1000 / 60);
	}
#else
	else if ( iName == "�݌v��" ) { oResult=int2zen(DwordToSystemTime( ::GetTickCount() - tick_count_at_load + tick_count_total ).wHour); }
	else if ( iName == "�݌v��" ) { oResult=int2zen(DwordToSystemTime( ::GetTickCount() - tick_count_at_load + tick_count_total ).wMinute); }
	else if ( iName == "�݌v�b" ) { oResult=int2zen(DwordToSystemTime( ::GetTickCount() - tick_count_at_load + tick_count_total ).wSecond); }
	else if ( iName == "�P���݌v�b" ) { oResult=int2zen( (::GetTickCount() - tick_count_at_load + tick_count_total)/1000 ); }
	else if ( iName == "�P���݌v��" ) { oResult=int2zen( (::GetTickCount() - tick_count_at_load + tick_count_total)/1000/60 ); }
#endif
	else if ( iName == "time_t" ) { time_t tm; time(&tm); oResult=itos(tm); }
	else if ( iName == "�ŏI�g�[�N����̌o�ߕb" ) { oResult=itos(second_from_last_talk); }

	else if ( compare_head(iName, "ResponseHistory") && aredigits(iName.c_str()+strlen("ResponseHistory")) ) {
	}

	else if ( compare_head(iName, "�T�[�t�F�X") && aredigits(iName.c_str()+10) ) {
		oResult=itos(cur_surface[ atoi(iName.c_str()+10) ]);
	}
	else if ( compare_head(iName, "�O��I�����T�[�t�F�X") && aredigits(iName.c_str()+20) ) {
		oResult=itos(last_talk_exiting_surface[ atoi(iName.c_str()+20) ]);
	}

	else if ( iName == "�ׂŋN�����Ă���S�[�X�g" ) { 
		updateGhostsInfo();	// �S�[�X�g�����X�V
		oResult = ( ghosts_info.size()>=2 ) ? (ghosts_info[1])["name"] : ""; 
	}
	else if ( iName == "�N�����Ă���S�[�X�g��" ) { 
		updateGhostsInfo();	// �S�[�X�g�����X�V
		oResult = itos(ghosts_info.size()); 
	}
	else if ( compare_head(iName, "isempty") && iName.size()>=8 ) {
		const char* p = iName.c_str()+7;
		mbinc(p);
		oResult = (*p=='\0') ? "1" : "0";
	}
	else if ( compare_head(iName, "���u") && compare_tail(iName, "�v�̑���") ) {
		string	str(iName, 4, iName.length()-4-8);
		oResult = talks.is_exist(str) ? "1" : "0";
	}
	else if ( compare_head(iName, "�ϐ��u") && compare_tail(iName, "�v�̑���") ) {
		string	str(iName, 6, iName.length()-6-8);
		oResult = (variables.find(str) != variables.end()) ? "1" : "0";
	}
	else if ( compare_head(iName, "�P��Q�u") && compare_tail(iName, "�v�̑���") ) {
		string	str(iName, 8, iName.length()-8-8);
		oResult = words.is_exist(str) ? "1" : "0";
	}
	else if ( compare_tail(iName, "�̑���") ) {
		updateGhostsInfo();	// �S�[�X�g�����X�V
		vector<strmap>::iterator i=ghosts_info.begin();
		for ( ; i!=ghosts_info.end() ; ++i )
			if ( compare_head(iName, (*i)["name"]) )
				break;
			else if ( compare_head(iName, (*i)["keroname"]) )
				break;
		oResult = ( i==ghosts_info.end() ) ? "0" : "1";
	}
	else if ( compare_tail(iName, "�̃T�[�t�F�X") ) {
		updateGhostsInfo();	// �S�[�X�g�����X�V
		vector<strmap>::iterator i=ghosts_info.begin();
		for ( ; i!=ghosts_info.end() ; ++i )
			if ( compare_head(iName, (*i)["name"]) ) {
				oResult = (*i)["sakura.surface"];
				break;
			} else if ( compare_head(iName, (*i)["keroname"]) ) {
				oResult = (*i)["kero.surface"];
				break;
			}

		if ( i==ghosts_info.end() ) {
			oResult = "-1";
		}
	}
	else if ( compare_head(iName, "FMO") && iName.size()>4 ) { // FMO?head
		updateGhostsInfo();	// �S�[�X�g�����X�V
		if ( !isdigit(iName[3]) )
			NULL;
		else if ( iName[3]-'0' > ghosts_info.size() )
			NULL;
		else {
			strmap&	m=ghosts_info[iName[3]-'0'];
			string	value(iName.c_str()+4);
			if ( m.find(value) != m.end() )
				oResult = m[value];
		}
	}
	else if ( compare_head(iName, "count") )
	{
		string	name(iName.c_str()+5);
		if ( name=="Words" ) { oResult = itos( words.size_of_family() ); }
		else if ( name=="Variable" ) { oResult = itos( variables.size() ); }
		else if ( name=="Anchor" ) { oResult = itos( anchors.size() ); }
		else if ( name=="Talk" ) { oResult = itos( talks.size_of_element() ); }
		else if ( name=="Word" ) { oResult = itos( words.size_of_element() ); }
		else if ( name=="NoNameTalk" )
		{
			Family<Talk>* f = talks.get_family("");
			oResult = itos( ( f==0 ) ? 0 : f->size_of_element() );
		}
		else if ( name=="EventTalk" )
		{
			int	n=0;
			for ( map< string, Family<Talk> >::const_iterator it = talks.compatible().begin() ; it != talks.compatible().end() ; ++it )
				if ( compare_head(it->first, "On") )
					n += it->second.size_of_element();
			oResult = itos(n);
		}
		else if ( name=="OtherTalk" )
		{
			int	n=0;
			for ( map< string, Family<Talk> >::const_iterator it = talks.compatible().begin() ; it != talks.compatible().end() ; ++it )
				if ( !compare_head(it->first, "On") && !it->first.empty() )
					n += it->second.size_of_element();
			oResult = itos(n);
		}
		else if ( name=="Line" )
		{
			int	n=0;
			for ( map< string, Family<Talk> >::const_iterator it = talks.compatible().begin() ; it != talks.compatible().end() ; ++it )
			{
				vector<const Talk*> v;
				it->second.get_elements_pointers(v);
				for ( vector<const Talk*>::const_iterator el_it = v.begin() ; el_it != v.end() ; ++el_it )
				{
					n += (*el_it)->size();
				}
			}
			for ( map< string, Family<Word> >::const_iterator it = words.compatible().begin() ; it != words.compatible().end() ; ++it )
			{
				n += it->second.size_of_element();
			}
			oResult = itos(n);
		}
		else if ( name=="Parenthesis" )
		{
			int	n=0;
			for ( map< string, Family<Talk> >::const_iterator it = talks.compatible().begin() ; it != talks.compatible().end() ; ++it )
			{
				vector<const Talk*> v;
				it->second.get_elements_pointers(v);
				for ( vector<const Talk*>::const_iterator el_it = v.begin() ; el_it != v.end() ; ++el_it )
				{
					for ( Talk::const_iterator tk_it = (*el_it)->begin() ; tk_it != (*el_it)->end() ; ++tk_it )
					{
						n += count(*tk_it, "�i");
					}
				}
			}
			for ( map< string, Family<Word> >::const_iterator it = words.compatible().begin() ; it != words.compatible().end() ; ++it )
			{
				vector<const Word*> v;
				it->second.get_elements_pointers(v);
				for ( vector<const Word*>::const_iterator el_it = v.begin() ; el_it != v.end() ; ++el_it )
				{
					n += count(**el_it, "�i");
				}
			}
			oResult = itos(n);
		}
	}
	else if ( iName=="���̃g�[�N" ) {
		map<int,string>::iterator it = reserved_talk.find(1);
		if ( it != reserved_talk.end() ) 
			oResult = it->second;
	}
	else if ( compare_head(iName,"������") && compare_tail(iName,"��ڂ̃g�[�N") ) {
		int	count = stoi( zen2han( string(iName.c_str()+6, iName.length()-6-12) ) );
		map<int,string>::iterator it = reserved_talk.find(count);
		if ( it != reserved_talk.end() ) 
			oResult = it->second;
	}
	else if ( compare_head(iName, "�g�[�N�u") && compare_tail(iName, "�v�̗\\x96\xf1�L��") ) { // �u��v�ɂ�\���܂܂��B
		string	str(iName, 8, iName.length()-8-12);
		oResult = "0";
		for (map<int, string>::iterator it=reserved_talk.begin(); it!=reserved_talk.end() ; ++it) {
			if ( str == it->second ) {
				oResult = "1";
				break;
			}
		}
	}
	else if ( iName == "�\\x96\xf1�g�[�N��" ) { // �u��v�ɂ�\���܂܂��B
		oResult = itos( reserved_talk.size() );
	}
	else if ( iName == "�C�x���g��" ) { oResult=mRequestID; }
	else if ( iName == "���O�̑I������" ) { oResult=last_choice_name; }
	else if ( mRequestMap.find(iName) != mRequestMap.end() ) {
		oResult = mRequestMap[iName];
	}
	else {
		// ������Ȃ������B�ʏ풝��H
		speaked_speaker.insert(speaker);
		characters += oResult.size();
		sender << "�i" << iName << "�j not found." << endl;
		return	false;
	}

	if ( p_kakko_replace_history!=NULL )
		p_kakko_replace_history->push_back(oResult);
	sender << "�i" << iName << "�j��" << oResult << "" << endl;
	return	true;
}