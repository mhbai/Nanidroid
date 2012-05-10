package com.cattailsw.nanidroid;

import android.app.Activity;
import android.os.Bundle;
import android.widget.ArrayAdapter;
import android.widget.ImageView;
import android.widget.Toast;
import android.os.Environment;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import android.widget.TextView;
import android.os.SystemClock;
import android.graphics.drawable.AnimationDrawable;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import java.util.Set;
import android.graphics.Rect;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.Color;
import java.util.Collections;
import java.util.Arrays;

import android.support.v4.app.DialogFragment;
import android.support.v4.app.FragmentActivity;
import android.util.Log;

import com.android.debug.hv.ViewServer;
import com.cattailsw.nanidroid.dlgs.AboutDialogFragment;
import com.cattailsw.nanidroid.dlgs.GhostListDialogFragment;
import com.cattailsw.nanidroid.dlgs.NoReadmeSwitchDlg;
import com.cattailsw.nanidroid.dlgs.NotImplementedDlg;
import com.cattailsw.nanidroid.dlgs.ReadmeDialogFragment;
import com.cattailsw.nanidroid.dlgs.EnterUrlDlg;
import com.cattailsw.nanidroid.dlgs.ErrMsgDlg;
import com.cattailsw.nanidroid.dlgs.NarPickDlg;
import com.cattailsw.nanidroid.dlgs.MoreGhostFuncDlg;
import com.cattailsw.nanidroid.util.AnalyticsUtils;
import com.cattailsw.nanidroid.util.NarUtil;
import com.cattailsw.nanidroid.util.PrefUtil;

import android.widget.FrameLayout;
import android.view.Gravity;
import android.content.Intent;
import java.util.List;
import android.net.Uri;
import android.widget.FrameLayout.LayoutParams;
import android.content.Intent;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.DialogInterface;
import android.content.res.AssetManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import java.io.BufferedReader;
import java.io.InputStreamReader;

public class Nanidroid extends FragmentActivity implements EnterUrlDlg.EUrlDlgListener,
							   NarPickDlg.NarPickDlgListener,
							   MoreGhostFuncDlg.MoreGhostFuncListener
{
    private static final String TAG = "Nanidroid";
    //private ImageView sv = null;
    private SakuraView sv = null;
    private KeroView kv = null;
    private Balloon bSakura = null;
    private Balloon bKero = null;
    private FrameLayout fl = null;

    private View btnBar = null;
    private View dbgBar = null;

    AnimationDrawable anime = null;
    //SurfaceManager mgr = null;
    LayoutManager lm = null;
    SScriptRunner runner = null;

    GhostMgr gm = null;
    List<InfoOnlyGhost> iglist = null;

    private static final String MIN_TAG = "minimized";
    private boolean restoreFromMinimize = false;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
	boolean dbgBuild = isDbgBuild();
        initGA();
	sv = (SakuraView) findViewById(R.id.sakura_display);
	kv = (KeroView) findViewById(R.id.kero_display);
	bSakura = (Balloon) findViewById(R.id.bSakura);
	bKero = (Balloon) findViewById(R.id.bKero);
	fl = (FrameLayout) findViewById(R.id.fl);
	btnBar = findViewById(R.id.btn_bar);
	dbgBar = findViewById(R.id.dbg_btn_bar);
	if ( dbgBuild )
	    dbgBar.setVisibility(View.VISIBLE);
	// need to get a list of ghosts on sd card
	if ( Environment.getExternalStorageState().equalsIgnoreCase(Environment.MEDIA_MOUNTED) == false ) {
	    bSakura.setText("sd card error");
	    return;
	    // need to prompt SD card issue
	}

	if ( savedInstanceState != null ) {
	    Log.d(TAG, "was minimized");
	    restoreFromMinimize = savedInstanceState.getBoolean(MIN_TAG, false);
	}

	//mgr = SurfaceManager.getInstance();
	lm = LayoutManager.getInstance(this);
	runner = SScriptRunner.getInstance(this);
	gm = new GhostMgr(this);
	if ( gm.getGhostCount() == 0 )
	    installFirstGhost(); // extract first to file dir on sd card
	
	String lastId = gm.getLastRunGhostId();
	if ( lastId == null ) lastId = "nanidroid";
	Ghost g = gm.createGhost(lastId);
	runner.setGhost(g);
	gm.setLastRunGhost(g);

	runner.setViews(sv, kv, bSakura, bKero);
	sv.setMgr(g.mgr);
	kv.setMgr(g.mgr);
	lm.setViews(fl, sv, kv, bSakura, bKero);
	runner.setLayoutMgr(lm);

	currentRunCount = getStartCount();
	if ( currentRunCount == 0 )
	    loadFirstRunScript();
	currentRunCount++;
	setStartCount(currentRunCount);

	Intent launchingIntent = getIntent();
	handleIntent(launchingIntent);

	updateSurfaceKeys(g);
	currentSurfaceKey = surfaceKeys[0];
	currentSurface = g.mgr.getSakuraSurface(currentSurfaceKey);
	//checkAndLoadAnimation();
	sv.changeSurface(currentSurfaceKey);
	kv.changeSurface("10");
	
	registerForContextMenu(findViewById(R.id.btn_help));
	
	NarUtil.createNarDirOnSDCard();

	ViewServer.get(this).addWindow(this);
    }

    private long currentRunCount = -1;

    private boolean isDbgBuild() {
	try {
	    boolean isDebuggable =  ( 0 != ( getPackageManager().getApplicationInfo("com.cattailsw.nanidroid", 
										    PackageManager.GET_META_DATA).flags &= ApplicationInfo.FLAG_DEBUGGABLE ) );
	    return isDebuggable;
	}
	catch(Exception e){
	    return false;
	}
    }
    
    private static final String PREF_KEY_LAUNCH_TIME = "keylaunchtime";

    private long getStartCount() {
	return PrefUtil.getKeyValueLong(getApplicationContext(), PREF_KEY_LAUNCH_TIME);
    }

    private void setStartCount(long count) {
	PrefUtil.setKey(getApplicationContext(), PREF_KEY_LAUNCH_TIME, count);
    }

    private void loadFirstRunScript() {
	try {
	    BufferedReader br = new BufferedReader(new InputStreamReader(getResources().openRawResource(R.raw.first_run_script), "UTF-8"));

	    for (String line = br.readLine(); line != null; line = br.readLine()) {
		if (line.equals("") || line.startsWith("#")) continue;
		runner.addMsgToQueue(new String[]{line});
	    }
	}
	catch(Exception e) {
	    runner.addMsgToQueue(new String[]{"\\0Oops, something wrong with first run script!\\e"});
	}
    }

    private void initGA(){
    	boolean enableGA = PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getBoolean(Setup.PREF_KEY_USE_ANALYTICS, true);
    	
    	AnalyticsUtils.getInstance(getApplicationContext(), Setup.UA_CODE, enableGA);
    	AnalyticsUtils.getInstance(getApplicationContext()).dispatch();
    }

    private void updateSurfaceKeys(Ghost g) {
	int keycount = g.mgr.getTotalSurfaceCount();
	surfaceKeys = new String[keycount];
	Set<String> k = g.mgr.getSurfaceKeys();
	surfaceKeys = k.toArray(surfaceKeys);
	Arrays.sort(surfaceKeys);
    }
 

    public void onPause() {
	super.onPause();
	if ( runner!= null ) { 
	    runner.stopClock();

	}
	sendStopIntent();
    }

    public void onDestroy() {
	super.onDestroy();
	ViewServer.get(this).removeWindow(this);
	sendStopIntent();
    }

    public void onResume() {
	super.onResume();
	if ( runner != null ) { 
	    runner.startClock();
	    runner.run();
	}
	AnalyticsUtils.getInstance(getApplicationContext()).trackPageView(TAG);
	ViewServer.get(this).setFocusedWindow(this);
    }

    public void onBackPressed() {
	if ( runner!= null ) { 
	    runner.stopClock();
	    runner.setCallback(mscb);
	    runner.stop();
	    runner.doExit();
	}
	else
	    super.onBackPressed();

    }

    private SScriptRunner.StatusCallback mscb = new SScriptRunner.StatusCallback() {
	    public void stop() {}
	    public void canExit() {
		runner.setCallback(null);
		finish();
	    }
	    public void ghostSwitchScriptComplete() {
		runner.setCallback(null);
		ghostSwitchStep2();
	    }
	};


    public void onWindowFocusChanged(boolean flag) {
	lm.checkAndUpdateLayoutParam();
    }

    private void checkAndLoadAnimation() {
	sv.changeSurface(currentSurfaceKey);
	kv.changeSurface("10");
	lm.checkAndUpdateLayoutParam();
	//checkAndUpdateLayoutParam();


	if (sv.hasAnimation() == false ){
	    findViewById(R.id.btn2).setEnabled(false);
	}
	else {
	    //iv.loadFirstAvailableAnimation();
	    animeIndex = currentSurface.getFirstAnimationIndex();
	    sv.loadAnimation(""+animeIndex);
	    /*anime = (AnimationDrawable) currentSurface.getAnimation(animeIndex, getResources());
	      anime.setVisible(true, true);
	      iv.setImageDrawable(anime);*/
	    findViewById(R.id.btn2).setEnabled(true);
	}

    }

    SurfaceReader sr = null;
    String[] surfaceKeys = null;
    int keyindex = 0;
    //Set<String> surfaceKeys = null;
    String currentSurfaceKey = null;
    ShellSurface currentSurface = null;

    public void onNextSurface(View v){

    	if ( keyindex < surfaceKeys.length - 1 )
	    keyindex++;
	else
	    keyindex = 0;
	
	currentSurfaceKey = surfaceKeys[keyindex];
	Log.d(TAG, "loading surface:" + currentSurfaceKey);
	currentSurface = sv.mgr.getSakuraSurface(currentSurfaceKey);
	bSakura.setText("current drawable key: " + currentSurfaceKey + 
			", animation count: " + currentSurface.getAnimationCount() +
			", collision count: " + currentSurface.getCollisionCount()
			);
	checkAndLoadAnimation();
    }
    int animeIndex = 0;

    public void onAnimate(View v) {
	//iv.setImageDrawable(anime);
	// 	anime.stop(); // stop previous animation?
	// 	anime.start();
	sv.startAnimation();
	
    }

    private void pickNextAnimation() {
	if ( currentSurface.getAnimationCount() > 1 ) {
	    animeIndex++;
	    if ( animeIndex >= currentSurface.getAnimationCount() )
		animeIndex = 0;

	    sv.loadAnimation(""+animeIndex);
	}
    }

    public void onShowCollision(View v){
	showCollisionAreaOnImageView();
    }


    public void ivClick(View v) {
	//anime.start();
	//showCollisionAreaOnImageView();
	pickNextAnimation();
	//runner.run();
    }

    private void showCollisionAreaOnImageView() {
	sv.showCollisionArea();
	kv.showCollisionArea();
    }

    public void runClick(View v){
	/*	String cmd ="\\habcdefghijklmnop\\uponmlkjihgfedcba\\h\\s[4]ksdjaklajdkasdjkl\\uasndklandklan\\s[300]\\nksjdklasjdk\\halalalsk\\e";
	  runner.addMsgToQueue(new String[]{cmd});
	  runner.run();*/
	//startService(new Intent(this, NanidroidService.class));	
	runner.clearMsgQueue();
    }

    private void sendStopIntent(){
	Intent i = new Intent(this, NanidroidService.class);
	i.setAction(NanidroidService.ACTION_CAN_STOP);
	startService(i);	
    }


    private void addNarToDownload(Uri target){
	Intent i = new Intent(this, NanidroidService.class);
	i.setAction(Intent.ACTION_RUN);
	i.setData(target);

	startService(i);	
    }
    
    public void narTest(View v){
	//extractNarTest();
	//addNarToDownload(Uri.parse("http://xx.xx.xxx/path/to/the/blab.nar"));
	//showReadme(new File("/mnt/sdcard/Android/data/com.cattailsw.nanidroid/files/ghost/mana/readme.txt"), "mana");
    	//extractNar("/mnt/sdcard/Android/data/com.cattailsw.nanidroid/cache/yumenikki.nar");
	//runOnUiThread(runner);//.startClock();
	//runner.startClock();
	//showCollisionAreaOnImageView();
    	extractNar("/mnt/sdcard/2elf-2.41.nar", true);
    }

    private void extractNar(String targetPath) {
    	extractNar(targetPath, false);
    }
    
    private void extractNar(String targetPath, boolean force){
	String ghostId = NarUtil.readNarGhostId(targetPath);

	if ( (gm.hasSameGhostId(ghostId) == false )|| force == true) {
	    if ( runner != null ) runner.doInstallBegin(ghostId);
		
	    InstallTask i = new InstallTask(targetPath, ghostId);
	    i.execute(targetPath);

	}
	else {
	    if ( runner != null )runner.doShioriEvent("OnInstallRefuse", null);
	    AnalyticsUtils.getInstance(this).trackEvent(Setup.ANA_ERR, "ghost_install", ghostId, -2);
	}
		
    }

    private void onSuccessGhostInstall(String ghostId, String gPath) {
	if (runner != null) runner.doInstallComplete(ghostId);
	AnalyticsUtils.getInstance(getApplicationContext()).trackEvent(Setup.ANA_PGM_FLOW, "ghost_install", ghostId, 1);

	// should show readme if one present
	//Log.d(TAG, "ghost:" + ghostId + " installed at:" + gPath);
	File readme = new File(gPath, "readme.txt");
	if (readme.exists()) {
	    showReadme(readme, ghostId);
	} else {
	    showGhostInstalledDlg(ghostId);
	}
    }
    
    private class InstallTask extends AsyncTask<String, Integer, String> {
	private String targetPath;
	private String ghostId;

    	InstallTask(String tPath, String gId) {
	    targetPath = tPath;
	    ghostId = gId;
    	}
    	
	@Override
	protected String doInBackground(String... params) {
	    String gPath = gm.installGhost(ghostId, targetPath);

	    return gPath;
	}
    	
	public void onPostExecute(String gPath) {
	    if ( gPath != null ) {
		onSuccessGhostInstall(ghostId, gPath);
	    }
	    else {
		if ( runner != null )runner.doShioriEvent("OnInstallFailure", null);
		AnalyticsUtils.getInstance(getApplicationContext()).trackEvent(Setup.ANA_ERR, "ghost_install", ghostId, -1);
	    }
	}
    }
    
    private void installFirstGhost(){
    	AssetManager a = getAssets();
    	try {
	    InputStream is = a.open("nanidroid.zip");
	    File extDir = getExternalCacheDir();
	    File targetPath = new File(extDir, "nanidroid.nar");
	    NarUtil.copyFile(is, new FileOutputStream(targetPath));
	    gm.installFirstGhost("nanidroid", targetPath.getPath());			
	} catch (IOException e) {
	    // TODO Auto-generated catch block
	    e.printStackTrace();
	}
    }

    private void showReadme(File readme, final String ghostId){
	AnalyticsUtils.getInstance(getApplicationContext()).trackPageView("/"+Setup.DLG_README+":ghostId");
        DialogFragment newFragment = ReadmeDialogFragment.newInstance(readme, ghostId);
        newFragment.show(getSupportFragmentManager(), Setup.DLG_README);
    }

    private void showGhostInstalledDlg(String ghostId){
	AnalyticsUtils.getInstance(getApplicationContext()).trackPageView("/"+Setup.DLG_NO_REAMDE+":ghostId");
    	DialogFragment f = NoReadmeSwitchDlg.newInstance(ghostId);
    	f.show(getSupportFragmentManager(), Setup.DLG_NO_REAMDE);
    }

    int cGindex = 0;
    public void onNextGhost(View v){
	String [] gname = gm.getGnames();// {"first","yohko","2elf"};
	if ( gname == null )
	    return;

	switchGhost(gname[cGindex]);
	cGindex++;
	if ( cGindex > gname.length -1)
	    cGindex = 0;
    }

    String nextGhostId = null;
    public void switchGhost(String nextId){
    	String nextName = gm.getGhostSakuraName(nextId);
    	if ( nextName == null ){
	    Log.d(TAG, "invalid next ghost id");
	    return;
    	}
    	nextGhostId = nextId;
	String nextPath = gm.getGhostPath(nextId);
    	runner.clearMsgQueue();
    	runner.setCallback(mscb);
    	runner.doGhostChanging(nextName, "manual", nextPath);
	AnalyticsUtils.getInstance(getApplicationContext()).trackEvent(Setup.ANA_PGM_FLOW,"ghost_switch",nextGhostId,0);

	//runner.doShioriEvent("OnGhostChanging", new String[]{g.getGhostName(), "manual", null, g.getGhostPath()});

    }

    public void ghostSwitchStep2() {
    	Ghost g;
    	try {
	    g = gm.createGhost(nextGhostId);
	    nextGhostId = null;
    	}
    	catch(Exception e) {
	    // TODO fill failed switch event!
	    AnalyticsUtils.getInstance(getApplicationContext()).trackEvent(Setup.ANA_ERR,"ghost_switch",nextGhostId,-1);
	    Log.d(TAG, "failed to switch to ghost:" + nextGhostId);
	    nextGhostId = null;
	    e.printStackTrace();
	    return;
    	}

	sv.setMgr(g.mgr);
	kv.setMgr(g.mgr);
	updateSurfaceKeys(g);
	
	keyindex = 0;
	currentSurfaceKey = surfaceKeys[keyindex];	
	sv.changeSurface(currentSurfaceKey);
	kv.changeSurface("10");
	lm.checkAndUpdateLayoutParam();
	gm.setLastRunGhost(g);
	runner.setGhost(g);
    }

    private void handleIntent(Intent intent){
	String action = intent.getAction();
	if (intent.hasExtra("DL_PKG")) {
	    Uri data = intent.getData();
	    bKero.setText("launching to extract nar at:" + data);

	    extractNar(data.getPath());// "/mnt/sdcard/2elf-2.41.nar");
	}
	if (action != null && action.equalsIgnoreCase(Intent.ACTION_VIEW)) {
	    // need to check the data?
	    Log.d(TAG, " action_view with data:" + intent.getData());
	    Uri target = intent.getData();
	    if (target != null)
		addNarToDownload(target);
	}
    }
    
    public void onNewIntent(Intent intent) {
	handleIntent(intent);
    }

    public void onUpdate(View v) {
	// need to show msg saying not implemented yet
	AnalyticsUtils.getInstance(getApplicationContext()).trackEvent(Setup.ANA_BTN, "Update", "", 0);
	NotImplementedDlg n = new NotImplementedDlg();
	n.show(getSupportFragmentManager(), Setup.DLG_NOT_IMPL);
    }
	
    public void onListGhost(View v){
	showGhostListDlg();
    }

    public void onHelp(View v) {
	AnalyticsUtils.getInstance(this).trackPageView("/Help_menu");
	openContextMenu(v);
    }
	
    public void getMoreGhost(int source) {
	AnalyticsUtils.getInstance(getApplicationContext()).trackEvent(Setup.ANA_BTN, "MoreGhost", 
								       source==0?"MainUI":Setup.DLG_G_LIST, source);
	/*		NotImplementedDlg n = new NotImplementedDlg();
	  n.show(getSupportFragmentManager(), Setup.DLG_NOT_IMPL);		*/
	//showUrlDlg();
	//startInstallFromSDCard();
	MoreGhostFuncDlg n = new MoreGhostFuncDlg();
	AnalyticsUtils.getInstance(this).trackPageView("/"+Setup.DLG_MORE_G);
	n.show(getSupportFragmentManager(), Setup.DLG_MORE_G);
    }

    public void startInstallFromSDCard() {
	String[] narz = NarUtil.listNarDir();
	if ( narz == null || narz.length == 0 ) {
	    // show error dlg
	    showNarErrDlg(narz==null);
	}
	else if ( narz.length > 1 ) {
	    showNarPickDlg(narz);
	}
	else {
	    extractNar(Environment.getExternalStorageDirectory() + "/nar/" + narz[0]);
	}
    }

    public void showNarErrDlg(boolean dir) {
	ErrMsgDlg e = ErrMsgDlg.newInstance(R.string.err_nar_title, 
					    dir?R.string.err_no_nar_folder:R.string.err_no_nar_file);
	e.show(getSupportFragmentManager(), Setup.DLG_ERR);
    }

    public void showNarPickDlg(String[] narz) {
	Toast.makeText(this, "multiple nar exist", Toast.LENGTH_SHORT).show();
	NarPickDlg n = new NarPickDlg(narz);
	AnalyticsUtils.getInstance(this).trackPageView("/"+Setup.DLG_NAR_PICK);
	n.show(getSupportFragmentManager(), Setup.DLG_NAR_PICK);
    }

    public void onNarPick(String narName) {
	extractNar(Environment.getExternalStorageDirectory() + "/nar/" + narName);	
    }

    public void showUrlDlg() {
	EnterUrlDlg u = new EnterUrlDlg();
	AnalyticsUtils.getInstance(this).trackPageView("/"+Setup.DLG_E_URL);
	u.show(getSupportFragmentManager(), Setup.DLG_E_URL);
    }

    public void onFinishURL(String url) {
	//Toast.makeText(this, "got url:" + url, Toast.LENGTH_SHORT).show();
	Uri targeturi = Uri.parse(url);
	addNarToDownload(targeturi);
    }

    public void showGhostTown() {
	AnalyticsUtils.getInstance(this).trackPageView("/ghost_town_portal");
	NotImplementedDlg n = new NotImplementedDlg();
	n.show(getSupportFragmentManager(), Setup.DLG_NOT_IMPL);
    }
	
    public void onMoreGhost(View v) {
	getMoreGhost(0);
    }
	
    private void showGhostListDlg() {
	String gn[] = gm.getGnames();
	gAdapter = new ArrayAdapter<String>(this,android.R.layout.simple_list_item_1, gn);
	
	AnalyticsUtils.getInstance(this).trackPageView("/"+Setup.DLG_G_LIST);
        DialogFragment newFragment = GhostListDialogFragment.newInstance(gn, gm);
        newFragment.show(getSupportFragmentManager(), Setup.DLG_G_LIST);	
    }
	
    static ArrayAdapter<String> gAdapter = null;

    @Override
    public boolean onContextItemSelected (MenuItem item) {
	int id = item.getItemId();
	switch(id) {
	case R.id.item_about:
	    showAbout();
	    return true;
	case R.id.item_feedback:
	    showFeedback();
	    return true;
	case R.id.item_general_help:
	    showHelp();
	    return true;
	}		
	return super.onOptionsItemSelected(item);
    }


    private void showHelp() {
	// TODO Auto-generated method stub
	Toast.makeText(this, "help clicked", Toast.LENGTH_SHORT).show();
    }

    private void showFeedback() {
		
	AnalyticsUtils.getInstance(getApplicationContext()).trackPageView("/feedback");
	Uri feedbackUri = Uri.parse(getString(R.string.feedback_url));
	startActivity(new Intent(Intent.ACTION_VIEW, feedbackUri));
    }

    private void showAbout() {
	AnalyticsUtils.getInstance(getApplicationContext()).trackPageView("/about");
	AboutDialogFragment f = new AboutDialogFragment();
	f.show(getSupportFragmentManager(), Setup.DLG_ABOUT);
    }
	
    @Override
    public void onCreateContextMenu(ContextMenu menu, View v,
				    ContextMenuInfo menuInfo) {
	super.onCreateContextMenu(menu, v, menuInfo);
		

	MenuInflater inflater = getMenuInflater();		
	inflater.inflate(R.menu.main_help_menu, menu);
    }

    public void onSetupClick(View v) {
	showPreference();
    }

    private void showPreference() {
	Intent intent = new Intent(Intent.ACTION_VIEW);

	intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET);
	intent.setClassName(Nanidroid.this, Preferences.class.getName());
	AnalyticsUtils.getInstance(this).trackPageView("/Preference");
	startActivity(intent);
    }

    public void frameClick(View v){
	//Toast.makeText(this, "frame touched", Toast.LENGTH_SHORT).show();
	// toggle main button bar visibility
	int vis = btnBar.getVisibility();
	if ( vis != View.VISIBLE ) {
	    AnalyticsUtils.getInstance(this).trackPageView("/main_btn_bar");
	    btnBar.setVisibility(View.VISIBLE);
	}
	else
	    btnBar.setVisibility(View.GONE);
    }
}
