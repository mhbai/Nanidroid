#include	"satori.h"	

int		Satori::CreateResponse(strmap& oResponse)
{
	// NOTIFYであれば値を保存
	/*strmap	mNotifiedMap;
	if ( mRequestCommand=="NOTIFY" )
		mNotifiedMap[mRequestID] = mRequestMap["Reference0"];*/
	if ( mRequestCommand=="NOTIFY" ) {
		if ( mRequestID=="hwnd" ) {
#ifndef POSIX
			strvec	vec;
			const int max = split(mReferences[0], byte1_dlmt, vec);
			for (int n=0 ; n<max ; ++n) {
				characters_hwnd[n] = (HWND)(stoi(vec[n]));
				sender << "里々は id:" << n << " のhWndを取得しました。" << endl;
			}
#endif
		}

	}
	string	result;

	if ( mRequestID == "OnDirectSaoriCall" ) {
		string	str;
		int	n=0;
		bool	isfirst = true;
		for (strvec::const_iterator i=mReferences.begin() ; i!=mReferences.end() ; ++i, ++n)
			if ( i->empty() && mRequestMap.find( string("Reference")+itos(n) )==mRequestMap.end() )
				break;
			else
				if ( isfirst ) {
					str = *i;
					isfirst = false;
				}
				else
					str += "," + *i;
		//while ( compare_tail(str, ",") )
		//	str.assign( str.substr(0, str.size()-2)+"]" );
		string	temp;
		if ( secure_flag ) {
			sender << "[DirectCall]" << endl;
			Call(str, temp);
			return	204;
		}
		else {
			sender << "local/Localでないので蹴りました: " << str << endl;
			return	204;
		}
	}
	else if ( compare_head(mRequestID, "On") ) {
		if ( words.is_exist(mRequestID) )
			return	Call(mRequestID, oResponse["Value"]) ? 200 : 204;
		return	EventOperation(mRequestID, oResponse);
	}
	else if ( mRequestID == "version" )
		result = gSatoriVersion;
	else if ( mRequestID == "craftman" )
		result = gSatoriCraftman;
	else if ( mRequestID == "craftmanw" )
		result = gSatoriCraftmanW;
	else if ( mRequestID == "name" )
		result = gSatoriName;
	else if ( 
		mRequestID=="sakura.recommendsites" || 
		mRequestID=="kero.recommendsites" ||
		mRequestID=="sakura.portalsites" ) 
	{
		if ( !GetURLList(mRequestID, result) )	// URLリストの取得
			return	204;
	} 
	else if ( !Call(mRequestID, result) ) {
		return	204;
	}
	else {
	}

	if ( result.empty() )
		return	204;

	oResponse["Value"] = result;
	return	200;
}
