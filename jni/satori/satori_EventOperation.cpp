#include	"satori.h"

inline bool	is_empty_script(const string& script) {
	for ( const char* p = script.c_str() ; *p!='\0' ; ) {
		if ( p[0]=='\\' ) {
			if ( p[1]=='0' || p[1]=='1' ) {
				p+=2;
				continue;
			}
			else if ( p[1]=='p' && p[2]=='[' ) {
				p+=3;
				while ( *p != ']' )
					if ( *p == '\0' )
						return	false;
					else
						++p;
				continue;
			}
		}
		return	false;
	}
	return	true;
}

int	Satori::EventOperation(string iEvent, map<string,string> &oResponse)
{
	// トーク先頭に「→」があると設定される。無ければ""のまま。
	mCommunicateFor="";
	// スクリプト文字列
	string	script="";

	// システムが欲しい情報を拾っておく
	if ( iEvent=="OnSecondChange" || iEvent=="OnMinuteChange" ) {

#ifndef POSIX
		if ( characters_hwnd[0]==NULL && updateGhostsInfo() && !ghosts_info.empty() ) {
			sender << "■FMOからhwndを取得しました。" << endl;
			characters_hwnd[0] = (HWND)(stoi( (ghosts_info[0])["hwnd"] )); 
		}

		//if ( !is_single_monitor && characters_hwnd[0]!=NULL ) {
		if ( is_single_monitor ) {
			//sender << "■シングルモニタです。見切れの独自判定を行いません。" << endl;
		}
		else if ( characters_hwnd[0]==NULL ) {
			//sender << "■マルチモニタですが、hwndが取得できていないため、見切れの独自判定を行いません。" << endl;

		}
		else {
		//	sender << "■見切れ判定処理" << endl;
			RECT	rc;
			::GetWindowRect(characters_hwnd[0], &rc);
			int	center = (rc.left + rc.right)/2;
			mRequestMap["Reference1"] = mReferences[1] =
				( center >= max_screen_rect.left && center <= max_screen_rect.right ) ? "0" : "1";
			/*sender << "シェルの左端: " << rc.left << endl;
			sender << "シェルの右端: " << rc.right << endl;
			sender << "シェルの中央: " << center << endl;
			sender << "デスクトップの左端: " << max_screen_rect.left << endl;
			sender << "デスクトップの右端: " << max_screen_rect.right << endl;
			sender << "■見切れ判定結果: " << mReferences[1] << endl;*/
		}
#endif

		mikire_flag = stoi(mReferences[1])!=0;
		kasanari_flag = stoi(mReferences[2])!=0;
		can_talk_flag = ( mReferences[3] != "0" );
		if ( iEvent[2]=='S' )
			++second_from_last_talk;
	}
	else if ( iEvent=="OnSurfaceChange" ) {
		cur_surface[0]=stoi(mReferences[0]);
		cur_surface[1]=stoi(mReferences[1]);
	} else if ( iEvent=="OnUpdateReady" ) {
		mReferences[0] = itos(stoi(mReferences[0])+1);
	}

	if ( (iEvent=="OnBoot" || iEvent=="OnGhostChanged") && !is_empty_script(on_loaded_script) ) {
		script = on_loaded_script;
		on_loaded_script = "";
	}
	else if ( iEvent=="OnClose" || iEvent=="OnGhostChanging" ) {
		string	on_unloading_script = GetSentence("OnSatoriClose");
		diet_script(on_unloading_script);
		if ( !is_empty_script(on_unloading_script) )
			script = on_unloading_script;
	}

	if ( !script.empty() ) {
	}
	else if ( iEvent=="OnAnchorSelect" && anchors.find(mReferences[0])!=anchors.end() ) {
		// OnAnchorSelectがきたとき、ref0がdicAnchorの文名の場合は
		// イベントが定義されている場合でもシステム側を優先する。
		script=GetSentence(mReferences[0]);
	}
	else if ( FindEventTalk(iEvent) ) {	// この際、互換イベントへの置換も同時に行われる
		// 定義されているならそれを優先する。
		script=GetSentence(iEvent);

		// これより以下は、イベントが定義されていない場合のデフォルト処理である。
	}
	else if ( iEvent=="OnMouseDoubleClick" )
	{
		static strvec v;
		if ( v.empty() )
		{
			v.push_back("＞（Ｒ３）（Ｒ４）つつかれ");
			v.push_back("（）");
		}
		script = SentenceToSakuraScript(v);
	}
	else if ( iEvent=="OnMouseMove" ) {
		nade_valid_time = nade_valid_time_initializer; // なでセッションの有効期限を更新
		int&	cur_nede_count = nade_count[ mReferences[4] ];
		if ( ++cur_nede_count >= nade_sensitivity ) {
#ifdef POSIX
		        int ret = 0;
#else
		        LRESULT	ret = 0;
			if ( !insert_nade_talk_at_other_talk && updateGhostsInfo() ) {
				string	hwnd_str = (ghosts_info[0])["hwnd"];
				HWND	hwnd = (HWND)(stoi(hwnd_str));
				if ( hwnd!=NULL ) {
					UINT	WM_SAKURAAPI = RegisterWindowMessage("Sakura");
					ret = ::SendMessage(hwnd, WM_SAKURAAPI, 140, 0);
				}
			}
#endif
			
			if ( ret == 0 ) {
				string	str = mReferences[3]+mReferences[4]+"なでられ";
				if ( talks.is_exist(str) )
					script=GetSentence(str);
				sender << "Talk: " << script << endl;
			}
			nade_count.clear();
		}
	}
	else if ( iEvent=="OnChoiceSelect" ) {
		script=GetSentence(mReferences[0]);
	}
	else if ( iEvent=="OnWindowStateRestore"
		|| iEvent=="OnShellChanged"
		|| iEvent=="OnSurfaceRestore"
		|| iEvent=="起動" ) {	// 「起動」は「OnBoot」「OnGhostChanged」からリダイレクトされる
		script=surface_restore_string();
	}
	else if ( iEvent=="終了" ) {
		script = "\\-";
	}
	else if ( iEvent=="OnMouseWheel" ) {
		koro_valid_time = 3;
		koro_count[ mReferences[4] ] += 1;
		if ( koro_count[ mReferences[4] ] >= 2 ) {
			string	str = mReferences[3]+mReferences[4]+"ころころ";
			if ( talks.is_exist(str) ) {
				script=GetSentence(str);
				koro_count.clear();
			}
		}
	}
	else if ( iEvent=="OnRecommendsiteChoice" )
	{
		if ( GetRecommendsiteSentence("sakura.recommendsites", script) )
			NULL;
		else if ( GetRecommendsiteSentence("kero.recommendsites", script) )
			NULL;
		else if ( GetRecommendsiteSentence("sakura.portalsites", script) )
			NULL;
		else
			script="";
	}
	else if ( iEvent=="OnCommunicate" ) {	// 話し掛けられた

		// ＄Sender	（if,（Ｒ０）==User,ユーザ,（Ｒ０））
		if ( mReferences[0]=="user" )
			mReferences[0]="ユーザ";

		// スクリプトに話者名を付加
		if ( !TalkSearch(mReferences[0]+"「"+mReferences[1]+"」", script, false) )
			script=GetSentence("COMMUNICATE該当なし");

		if ( mCommunicateFor==""  ) {	// 手動打ち切り
			sender << "里々COMMUNICATE、辞書に続行指示が無いことによる打ち切り" << endl;
			mCommunicateLog.clear();
		}
		else if ( mCommunicateLog.find(script) != mCommunicateLog.end() ) {
			sender << "里々COMMUNICATE、自分側ループにより打ち切り" << endl;
			script="";	// 何も言わない
			mCommunicateLog.clear();
			mCommunicateFor = "";
		}
		else  if ( mCommunicateLog.find(mReferences[1]) != mCommunicateLog.end() ) {
			sender << "里々COMMUNICATE、相手側ループにより打ち切り" << endl;
			mCommunicateLog.clear();
			mCommunicateFor = "";
		} 
		else {	// 続行
			sender << "里々COMMUNICATE、続行" << endl;
			mCommunicateLog.insert(mReferences[1]);
			mCommunicateLog.insert(script);
		}
	}

	if ( iEvent=="OnSecondChange" ) {

		// 存在するタイマのディクリメント
		for (strintmap::iterator i=timer.begin();i!=timer.end();++i)
			variables[i->first + "タイマ"] = int2zen( --(i->second) );

		// 自動セーブ
		if ( mAutoSaveInterval > 0 ) {
			if ( --mAutoSaveCurrentCount <= 0 ) {
				this->Save(false);
				mAutoSaveCurrentCount = mAutoSaveInterval;
			}
		}
	}

	diet_script(script);
	if ( is_empty_script(script) && can_talk_flag && iEvent=="OnSecondChange" ) {

		// タイマ予約発話
		for (strintmap::iterator i=timer.begin();i!=timer.end();++i) {
			if ( i->second < 1 ) {
				string	var_name = i->first + "タイマ";
				sender << var_name << "が発動。" << endl;
				script=GetSentence(i->first);
				timer.erase(i);
				variables.erase(var_name);
				if ( !is_empty_script(script) )
					diet_script(script);
				break;
			}
		}

		// 自動発話
		if ( is_empty_script(script) && !mikire_flag ) {
			if ( nade_valid_time>0 )
				if ( --nade_valid_time == 0 )
					nade_count.clear();
			if ( koro_valid_time>0 )
				if ( --koro_valid_time == 0 )
					koro_count.clear();
			if ( talk_interval>0 && --talk_interval_count<0 ) {
				string	iEvent="OnTalk";
				FindEventTalk(iEvent);
				script=GetSentence(iEvent);
				diet_script(script);
			}
		}
	}

	if ( is_empty_script(script) )
		return	204;	// 喋らない

	// scriptへの付与処理
	if ( is_speaked_anybody() ) {
		script = surface_restore_string() + append_at_talk_start + script + append_at_talk_end;

		// 喋りカウント初期化
		int	dist = int(talk_interval*(talk_interval_random/100.0));
		talk_interval_count = ( dist==0 ) ? talk_interval : 
			(talk_interval-dist)+(random()%(dist*2));
	}
	script += ( iEvent=="OnClose" || iEvent=="終了" ) ? "\\-" : "\\e";


	if ( !mCommunicateFor.empty() ) { // 話しかけの有無
		sender << "里々COMMUNICATE、「" << mCommunicateFor << "」へ話し掛け" << endl;
		oResponse["Reference0"] = mCommunicateFor;
		oResponse["To"] = mCommunicateFor;
		if ( iEvent!="OnCommunicate" )
		{
			mCommunicateLog.clear(); // 初回のみ。続行時にはここでクリアはしない。
		}
		//mCommunicateLog.insert(script);
	}
	oResponse["Value"]=script;

	// １トーク中でのみ有効な重複回避をクリア
	words.handle_talk_end();
	talks.handle_talk_end();

	// バルーン位置が有効なら設定
	if ( validBalloonOffset[0] && validBalloonOffset[1] )
		oResponse["BalloonOffset"]=string()+BalloonOffset[0]+byte1_dlmt+BalloonOffset[1];

	return	200;
}



//		string	iEvent="OnTalk";
//		FindEventTalk(iEvent);
//		script = surface_restore_string() + append_at_talk_start + GetSentence(iEvent) + append_at_talk_end + "\\e";



// Communicate形式検索。該当ありならそのスクリプトを取得、該当なしならfalse。
bool	Satori::TalkSearch(const string& iSentence, string& oScript, bool iAndMode)
{
	const Talk* talk = talks.communicate_search(iSentence, iAndMode);
	if ( talk == NULL )
	{
		return false;
	}

	oScript = SentenceToSakuraScript(*talk);
	sender << oScript << endl;
	return	true;
}


