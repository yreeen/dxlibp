DX Library Portable	Ver0.5.0
開発者：憂煉

これはPSPの開発を簡単に行うためのライブラリです。
詳しい事はホームページのWiki等を漁ってください。
http://dxlibp.sourceforge.jp/

【注意！】このライブラリを使用してゲームを作成した場合、次の著作権表示を必ず含めてください。

【著作権表示必須項目】

zlib
Copyright (c) 1995-2004 Jean-loup Gailly and Mark Adler.
※ただしLoadGraph等の画像読み込み関数を使っていない場合は無しで大丈夫です。

intraFont
Uses intraFont by BenHur
※ただしDrawString等の文字列描画関数を使っていない場合は無しで大丈夫です。

liblzr
Uses liblzr by BenHur


全角文字表示ライブラリ
このソフトウェアはmediumgauge氏作成の全角文字表示ライブラリを使用しています。
※ただしprintfDx等の簡易文字列描画関数を使っていない場合は無しで大丈夫です。

【必須じゃないけど入れてくれたら嬉しい項目】

DXライブラリPortable
DX Library Portable Copyright (C) 2008-2009 Kawai Yuichi.

【0.4.x以前からの更新注意！】
依存しているライブラリが変更されたので、Makefileに指定するLIBSの項目が変化しています。
LIBS = dxlibp.a -lpspgu -lz -lm -lpsprtc -lpspaudio -lpspaudiocodec -lstdc++ -lpsputility -lpspvalloc -lpsppower

【更新履歴】

Ver0.0.0		公開開始
Ver0.0.1		恥ずかしいバグを修正
Ver0.0.2		高速化＆バグ修正
Ver0.1.0		Sliceの導入、DeleteGraph関数を作り忘れていたので追加、TransFlagを使えるようにした、ConvertGraphFormat関数を追加、その他バグ修正
Ver0.1.1		DrawRotaGraphの計算に恥ずかしいミスがあったので修正。全ての画像描画関数でSliceを利用するように変更
Ver0.2.0		MP3ファイルをPlayMusicで再生できるようにしました。WAVは再生できないので注意です
Ver0.2.1		printfDx系関数を全面書き直ししました。DrawString_Shinonome関数を追加しました。
Ver0.3.0		GetTextOSK関数を追加しました。
Ver0.3.1		ファイルの取り扱いに関する致命的なバグを修正
Ver0.3.2		libjpegとPSP内部のJpegデコーダのどちらを使うかをDXPコンパイル時に選択可能にした。
Ver0.4.0		intraFontを内包し、PSP本体に組み込まれているフォントを利用可能にした。音楽再生機能の再構築をした。
Ver0.4.1		DrawExtendGraphで表示結果が反転する時とDrawTurnGraph使用時で描画結果がおかしくなる問題を修正。（前記に該当する場合は内部的にDrawModiGraphを使うため若干の速度低下が発生します。
Ver0.4.2		GetAnalogInputが不正な値を返す問題を修正。
Ver0.4.3		LoadDivGraph関数とDerivationGraph関数を実装。
Ver0.4.4		内部構造の一新とSetDrawScreen関数の追加。SetDrawScreenに渡すグラフィックハンドルはVRAM上にある必要があります。
Ver0.4.5		音楽再生機能のバグ修正（シューティング大好き氏によるソースコード改修）、描画ルーチンの高速化などです。
Ver0.4.6		しょうもないミスタイプで描画結果がおかしくなっていたのを修正
Ver0.4.7
Ver0.4.8		バグ修正
Ver0.4.9		ブレンドモードの仕様を本家に合わせた。
Ver0.4.10		加算合成が正常に働かないバグを修正
Ver0.4.11		パレット形式のPNG画像はパレットを使って描画するように修正
Ver0.4.12		MP3の再生を安定化、文字列描画のバグ修正、その他いろいろ
Ver0.4.14		描画機能を更に高速化
Ver0.5.0		ソースコード全体をほとんど書き直した。