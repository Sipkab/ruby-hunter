/*
 * Copyright (C) 2020 Bence Sipka
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package bence.sipka.graphics.framework;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;

import android.annotation.TargetApi;
import android.app.Application;
import android.app.NativeActivity;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.hardware.display.DisplayManager;
import android.hardware.display.DisplayManager.DisplayListener;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.InputType;
import android.util.DisplayMetrics;
import android.util.SparseArray;
import android.view.Display;
import android.view.Gravity;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.CompletionInfo;
import android.view.inputmethod.CorrectionInfo;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputContentInfo;
import android.view.inputmethod.InputMethodManager;

@TargetApi(Build.VERSION_CODES.GINGERBREAD)
public class MainNativeActivity extends NativeActivity {
	private static final int DEFAULT_INPUT_TYPE = InputType.TYPE_TEXT_VARIATION_VISIBLE_PASSWORD | InputType.TYPE_CLASS_TEXT
			| InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS;

	private static class SoftKeyboardView extends View {
		private int inputType = DEFAULT_INPUT_TYPE;
		private int options = 0;

		public SoftKeyboardView(Context context) {
			super(context);
			setFocusableInTouchMode(true);
			if (android.os.Build.VERSION.SDK_INT >= 11) {
				options |= EditorInfo.IME_FLAG_NO_FULLSCREEN;
			}
		}

		@Override
		public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
			outAttrs.inputType = inputType;
			outAttrs.imeOptions = EditorInfo.IME_FLAG_NO_EXTRACT_UI | EditorInfo.IME_ACTION_DONE | options;
			return new BaseInputConnection(this, false) {

				@Override
				public boolean deleteSurroundingText(int beforeLength, int afterLength) {
					// bad workaround for google soft keyboard :-(
					// it sends before-after 1-0 or 2-0 instead of backspace KEYDOWN and KEYUP events
					// sends 2-0 if unicode char was entered before
					// related: https://code.google.com/p/android/issues/detail?id=42904
					// http://stackoverflow.com/questions/18581636/android-cannot-capture-backspace-delete-press-in-soft-keyboard
					if (beforeLength > 0) {
						// sendKeyEvent returns false all the time
						super.sendKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_DEL));
						super.sendKeyEvent(new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_DEL));
						return true;
					}
					return super.deleteSurroundingText(beforeLength, afterLength);
				}

				@Override
				public boolean commitCompletion(CompletionInfo text) {
					// TODO Auto-generated method stub
					System.out.println("MainNativeActivity.SoftKeyboardView.onCreateInputConnection(...).new BaseInputConnection() {...}.commitCompletion()");
					return super.commitCompletion(text);
				}

				@Override
				public boolean commitContent(InputContentInfo inputContentInfo, int flags, Bundle opts) {
					// TODO Auto-generated method stub
					System.out.println("MainNativeActivity.SoftKeyboardView.onCreateInputConnection(...).new BaseInputConnection() {...}.commitContent()");
					return super.commitContent(inputContentInfo, flags, opts);
				}

				@Override
				public boolean commitCorrection(CorrectionInfo correctionInfo) {
					// TODO Auto-generated method stub
					System.out.println("MainNativeActivity.SoftKeyboardView.onCreateInputConnection(...).new BaseInputConnection() {...}.commitCorrection()");
					return super.commitCorrection(correctionInfo);
				}

				@Override
				public boolean commitText(CharSequence text, int newCursorPosition) {
					// TODO Auto-generated method stub
					System.out.println("MainNativeActivity.SoftKeyboardView.onCreateInputConnection(...).new BaseInputConnection() {...}.commitText()");
					return super.commitText(text, newCursorPosition);
				}
			};
		}
	}

	private static final int MSG_DRAW = 0;

	// https://code.google.com/p/android/issues/detail?id=206648
	private static class DrawHandler extends Handler {
		private MainNativeActivity activity;

		public DrawHandler(MainNativeActivity activity) {
			this.activity = activity;
		}

		@Override
		public void handleMessage(Message msg) {
			activity.nativeExecuteDraw(activity.nativePointer);
			sendEmptyMessage(MSG_DRAW);
		}
	}

	private boolean imeVisible;
	private InputMethodManager imm;

	private long nativePointer;

	private SparseArray<KeyCharacterMap> charmap = new SparseArray<KeyCharacterMap>();

	private ByteBuffer unicodeKeyBuffer = null;

	private DrawHandler handler = new DrawHandler(this);

	private float xdpi;
	private float ydpi;

	private SoftKeyboardView keyboardView;

	private static Display defaultDisplay;
	private static Application applicationInstance;

	private native void handleMultipleUnicodeInput(long nativeptr, ByteBuffer buffer, int count);

	private native void nativeExecuteDraw(long nativeptr);

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		applicationInstance = getApplication();
		defaultDisplay = getWindowManager().getDefaultDisplay();
		DisplayMetrics dm = getResources().getDisplayMetrics();
		xdpi = dm.xdpi;
		ydpi = dm.ydpi;

		super.onCreate(savedInstanceState);

		imm = (InputMethodManager) this.getSystemService(INPUT_METHOD_SERVICE);
		// getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);

		keyboardView = new SoftKeyboardView(this);

		addContentView(keyboardView, new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));

	}

	@Override
	protected void onStart() {
		super.onStart();
		handler.sendEmptyMessage(MSG_DRAW);
	}

	@Override
	protected void onResume() {
		super.onResume();
	}

	@Override
	protected void onPause() {
		super.onPause();
	}

	@Override
	protected void onStop() {
		handler.removeMessages(MSG_DRAW);
		super.onStop();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		imm = null;
		unicodeKeyBuffer = null;
		charmap = null;
	}

	@Override
	public boolean dispatchKeyEvent(KeyEvent event) {
		// native code did not handle KeyEvent

		if (event.getAction() == KeyEvent.ACTION_MULTIPLE) {
			String characters = event.getCharacters();
			if (characters != null) {
				final int codePointCount = characters.codePointCount(0, characters.length());
				ByteBuffer buffer = getBufferForCount(codePointCount);

				final int length = characters.length();
				for (int offset = 0; offset < length;) {
					final int codepoint = characters.codePointAt(offset);

					buffer.putInt(codepoint);

					offset += Character.charCount(codepoint);
				}

				handleMultipleUnicodeInput(nativePointer, buffer, codePointCount);
				return true;
			}
		}

		return super.dispatchKeyEvent(event);
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus && imeVisible) {
			requestKeyboard();
			// showAdPopup();
		}
	}

	private void requestKeyboard() {
		imeVisible = true;

		imm.showSoftInput(keyboardView, InputMethodManager.SHOW_FORCED);
	}

	public void requestKeyboard(int type) {
		int ntype;
		switch (type) {
			case 0: {
				ntype = DEFAULT_INPUT_TYPE;
				break;
			}
			case 1: {
				ntype = InputType.TYPE_CLASS_NUMBER;
				break;
			}
			default: {
				throw new RuntimeException("Invalid type: " + type);
			}
		}
		if (ntype != keyboardView.inputType) {
			keyboardView.inputType = ntype;
			imm.restartInput(keyboardView);
		}
		requestKeyboard();
	}

	public void dismissKeyboard() {
		imeVisible = false;
		imm.hideSoftInputFromWindow(keyboardView.getWindowToken(), 0);
	}

	public final int getUnicodeFromKey(int keycode, int metastate, int deviceid) {
		try {
			KeyCharacterMap map = charmap.get(deviceid);

			if (map == null) {
				map = KeyCharacterMap.load(deviceid);
			}
			return map.get(keycode, metastate);
		} catch (Exception e) {
			// TODO why do we need general Exception here?
			// catch UnavailableException if no device exists
			e.printStackTrace();
		}
		return 0;
	}

	private final ByteBuffer getBufferForCount(int count) {
		if (unicodeKeyBuffer == null || unicodeKeyBuffer.capacity() < count * 4) {
			unicodeKeyBuffer = ByteBuffer.allocateDirect((count + 4) * 4).order(ByteOrder.nativeOrder());
		} else {
			unicodeKeyBuffer.rewind();
		}
		return unicodeKeyBuffer;
	}

	// @Override
	// public boolean onKeyDown(int keyCode, KeyEvent event) {
	// System.out.println("MainNativeActivity.onKeyDown()");
	// InputMethodManager imm = (InputMethodManager) this.getSystemService(INPUT_METHOD_SERVICE);
	// System.out.println("active: " + imm.isActive(getWindow().getDecorView()));
	// System.out.println(getWindow().getDecorView());
	// imm.showSoftInput(getWindow().getDecorView(), InputMethodManager.SHOW_IMPLICIT);
	// return true;
	// }

	private void showAdPopup() {
		View view = new View(MainNativeActivity.this);
		view.setBackgroundColor(Color.WHITE);

		WindowManager wm = (WindowManager) getSystemService(WINDOW_SERVICE);
		WindowManager.LayoutParams params = new WindowManager.LayoutParams();
		params.width = 720;
		params.height = 200;
		params.gravity = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
		params.flags = WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE;
		wm.addView(view, params);

	}

	/**
	 * Called from native side, when the activity pointer is available.
	 * 
	 * @param nativeactivitypointer
	 *            the native pointer to the activity data
	 */
	public void setNativePointer(long nativeactivitypointer) {
		this.nativePointer = nativeactivitypointer;
	}

	public interface ActivityResultListener {
		public boolean handleActivityResult(int requestCode, int resultCode, Intent data);
	}

	private List<ActivityResultListener> activityResultListeners = null;

	public synchronized void addActivityResultListener(ActivityResultListener listener) {
		if (activityResultListeners == null) {
			activityResultListeners = new ArrayList<MainNativeActivity.ActivityResultListener>();
		}
		activityResultListeners.add(listener);
	}

	public synchronized void removeActivityResultListener(ActivityResultListener listener) {
		if (activityResultListeners == null) {
			activityResultListeners = new ArrayList<MainNativeActivity.ActivityResultListener>();
		}
		activityResultListeners.remove(listener);
	}

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		synchronized (this) {
			if (activityResultListeners != null) {
				for (ActivityResultListener l : activityResultListeners) {
					if (l.handleActivityResult(requestCode, resultCode, data)) {
						return;
					}
				}
			}
		}
		super.onActivityResult(requestCode, resultCode, data);
	}

	public static int getDisplayRotationDegrees() {
		return getDisplayRotationDegrees(defaultDisplay);
	}

	private static int getDisplayRotationDegrees(Display d) {
		return d.getRotation() * 90;
	}

	public static void createLooper() {
		Looper.prepare();
	}

	/**
	 * The currently active DisplayManager.DisplayListener.<br>
	 * Using Object as type, so this doesn't break API compatibility below level 17.
	 */
	private static Object displayListener = null;

	private static native void deviceRotationChanged(int degrees);

	/**
	 * Called from native, to subscribe to Display orientation changes. Min API level is 17.
	 */
	@TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
	private static int subscribeOrientation() {
		final class NativeDisplayListener implements DisplayManager.DisplayListener {
			private int oldRotation = -1;
			private int displayId = defaultDisplay.getDisplayId();

			private NativeDisplayListener() {
			}

			@Override
			public void onDisplayAdded(int displayId) {
			}

			@Override
			public void onDisplayChanged(int displayId) {
				if (displayId != this.displayId) {
					return;
				}
				int nrot = getDisplayRotationDegrees();
				if (oldRotation != nrot) {
					oldRotation = nrot;
					deviceRotationChanged(nrot);
				}
				android.util.Log.i("MainNativeActivity", "Display #" + displayId + " changed. " + getDisplayRotationDegrees());
			}

			@Override
			public void onDisplayRemoved(int displayId) {
			}
		}

		DisplayManager displaymanager = (DisplayManager) applicationInstance.getSystemService(Context.DISPLAY_SERVICE);
		NativeDisplayListener listener = new NativeDisplayListener();

		displaymanager.registerDisplayListener(listener, new Handler(Looper.getMainLooper()));
		displayListener = listener;

		final int oldrot = getDisplayRotationDegrees();
		listener.oldRotation = oldrot;
		return oldrot;
	}

	/**
	 * Called from native, to unsibscrube from Display orientation changes. Min API level is 17.
	 */
	@TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
	private static void unsubscribeOrientation() {
		DisplayManager displaymanager = (DisplayManager) applicationInstance.getSystemService(Context.DISPLAY_SERVICE);
		displaymanager.unregisterDisplayListener((DisplayListener) displayListener);
		displayListener = null;
	}

	public void requestAppRating() {
		String packagename = getPackageName();

		Uri uri = Uri.parse("market://details?id=" + packagename);
		Intent intent = new Intent(Intent.ACTION_VIEW, uri);

		// To count with Play market backstack, After pressing back button,
		// to taken back to our application, we need to add following flags to intent.
		int flags = Intent.FLAG_ACTIVITY_NO_HISTORY | Intent.FLAG_ACTIVITY_MULTIPLE_TASK;
		if (android.os.Build.VERSION.SDK_INT >= 21) {
			flags |= Intent.FLAG_ACTIVITY_NEW_DOCUMENT;
		} else {
			flags |= Intent.FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET;
		}
		intent.addFlags(flags);
		try {
			startActivity(intent);
		} catch (ActivityNotFoundException e) {
			startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("http://play.google.com/store/apps/details?id=" + packagename)));
		}
	}

}
