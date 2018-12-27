# RemoteTalk
VOICEROID や CeVIO Creative Studio などの音声を Unity の AudioSource として使えるようにするプラグインです。  
パラメータの設定、テキストの入力、読み上げを Unity 側からコントロールでき、読み上げた音声は自動的にファイル (wav, ogg) にエクスポートできます。また、Timeline との強力な連携機能も備えています。

現在動作の確認が取れているツールは以下になります。VOICEROID シリーズは他のキャラクターでも動作する可能性は高いと思われますが、未確認です。
- VOICEROID2 (Version 2.0.4.0 結月ゆかり、継星あかり)
- VOICEROID EX+ (Version 1.7.3 民安ともえ=弦巻マキ)
- CeVIO Creative Studio 6 (Version 6.1.29.0)
- Windows SAPI (Windows に標準搭載されている読み上げ機能)

Unity は 5.6 以降で動作を確認しています。

## 基本的な使い方
<img align="right" src="https://user-images.githubusercontent.com/1488611/50464656-8379ea80-09d5-11e9-838e-b88579d372a5.png" width=250>

- RemoteTalk.unitypackage をプロジェクトにインポート
- GameObject -> RemoteTalk -> Create Client でクライアントを作成
- RemoteTalkClient の "Connection" から対応ツールに接続
  - VOICEROID2: ボタンを押せば自動的に起動するはずですが、ツールを検出できなかった場合は VoiceroidEditor.exe の場所を指定するためのダイアログを出します。既に起動していた場合は起動中のツールへ接続します
  - VOICEROID Ex の場合、ダイアログが開くので、対象の .exe ファイルを選択します
  - CeVIO CS: ボタンを押せば自動的に起動します。既に起動していた場合は起動中のツールへ接続します
- 接続後はテキストを入力して "Play" で再生＆エクスポートが可能です。

## Timeline 連動
本プラグインの真価は Timeline との連動にあります。

- Timeline ウィンドウの "Add" から Ist.RemoteTalk -> Remote Talk Track で専用の Track を追加します
- Clip は複数選択による同時編集が可能です。キャストの一括変更、読み上げ速度の一括変更などが簡単にできます
- 一度再生した音声はファイルにエクスポートされ、以降はそのファイルを再生するようになります (=再生に外部ツールは不要になります)
  - 音声ファイルはキャスト、テキスト、パラメータに基づいて生成されます。つまり、同じキャストとテキストでも、パラメータが異なれば別ファイルになります
- **"Convert To Audio Track" で AudioTrack に変換が可能です**。全て AudioTrack 化しておけば、ランタイムでは本プラグインは完全に不要となります。また、RemoteTalkTrack がサポートしていない blending なども AudioTrack 化すれば可能になります。

## 注意点
リアルタイムの読み上げには、対応ツールがインストールされている必要があります。本プラグインを使用したプロジェクトをビルドして配布する場合、実行する側でもツールが必要になります。  
事前に音声ファイルにエクスポートしていた場合は再生する側でツールは不要になりますが、この場合はライセンス規約に抵触しないように注意が必要です。**VOICEROID も CeVIO CS も、営利目的の使用には別途有償のライセンス契約が必要になります**。一部例外もあります。詳しくは各製品の商用利用に関するページを参照ください:
- [VOICEROID: 個人向け商用ライセンス](https://www.ah-soft.com/licensee/voice_individual.html)
- [CeVIO: 音声データやキャラクターの利用について](http://cevio.jp/commercial/)
  
また、音声素材のみの公開も制限されています。例えば、エクスポートした音声ファイルを含むプロジェクトを github で公開する、といった行為は避けたほうがいいでしょう。


## License
[MIT](LICENSE.txt)
