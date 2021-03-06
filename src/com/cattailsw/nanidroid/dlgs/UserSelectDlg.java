package com.cattailsw.nanidroid.dlgs;

import android.support.v4.app.DialogFragment;
import android.os.Bundle;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import com.cattailsw.nanidroid.R;

public class UserSelectDlg extends DialogFragment {

    public interface UserSelDlgListener {
	public void onChoiceSelect(String id);
    }

    public static UserSelectDlg newInstance(String[] text, String[] ids) {
	UserSelectDlg frag = new UserSelectDlg();
	frag.labels = text;
	frag.idz = ids;

	return frag;
    }

    String[] labels;
    String[] idz;

    @Override
	public void onSaveInstanceState(Bundle arg0) {
    	arg0.putStringArray("labels", labels);
    	arg0.putStringArray("idz", idz);
		super.onSaveInstanceState(arg0);
	}

	@Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
		if ( labels == null ) labels = savedInstanceState.getStringArray("labels");
		if ( idz == null ) idz = savedInstanceState.getStringArray("idz");
		
            return new AlertDialog.Builder(getActivity())
		.setCancelable(false)
		.setTitle(R.string.user_sel_dlg_title)
		.setItems(labels,
			  new OnClickListener() {
			      public void onClick(DialogInterface dlg, int which) {
				  if( which < labels.length) {
				      ((UserSelDlgListener) getActivity()).onChoiceSelect(idz[which]);
				  }
			      }
			  })
		.create();
    }
}
