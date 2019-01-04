[![RemoteTalk demo](https://img.youtube.com/vi/QeWlsMqsYtg/0.jpg)](https://www.youtube.com/watch?v=QeWlsMqsYtg)  
[RemoteTalk demo - https://www.youtube.com/watch?v=QeWlsMqsYtg](https://www.youtube.com/watch?v=QeWlsMqsYtg)
# RemoteTalk
VOICEROID や CeVIO Creative Studio などの音声を Unity の AudioSource として使えるようにするプラグインです。  
パラメータの設定、テキストの入力、読み上げを Unity 側からコントロールでき、読み上げた音声は自動的にファイル (wav, ogg) にエクスポートできます。また、Timeline との強力な連携機能も備えています。

現在動作の確認が取れているツールは以下になります。VOICEROID シリーズは他のキャラクターでも動作する可能性は高いと思われますが、未確認です。
- VOICEROID2: Version 2.0.4.0 - 2.0.5.0 + 結月ゆかり、継星あかり、民安ともえ(EX からのインポート)
- VOICEROID EX+: Version 1.7.3 民安ともえ
- CeVIO Creative Studio: Version 6.1.29.0
- Windows SAPI (Windows に標準搭載されている読み上げ機能)

Unity は 5.6 以降で動作します。ただし、5.6 には Timeline がありません。2017 系以降は Timeline が使えますが、2017 系と 2018 系で動作に若干違いがあります。2018 の方が望ましい動作になります。

話者/キャラクター のことを以降 "キャスト" と表記します。

## 基本的な使い方
<img align="right" src="https://user-images.githubusercontent.com/1488611/50464656-8379ea80-09d5-11e9-838e-b88579d372a5.png" width=300>

- [Releases](https://github.com/i-saint/RemoteTalk/releases) から RemoteTalk.unitypackage をダウンロードしてプロジェクトにインポート
  - Unity 2018.3 以降の場合、このリポジトリを直接インポートすることもできます。プロジェクト内にある Packages/manifest.json をテキストエディタで開き、"dependencies" に以下の行を加えます。
  > "com.i-saint.remotetalk": "https://github.com/i-saint/RemoteTalk.git",

- GameObject -> RemoteTalk -> Create Client でクライアントを作成
- RemoteTalkClient の "Connection" からツールに接続
  - VOICEROID2: ボタンを押せば自動的に起動するはずですが、ツールを検出できなかった場合は VoiceroidEditor.exe の場所を指定するためのダイアログを出します。起動済みの場合は起動中のツールへ接続します。
  - VOICEROID Ex: ファイルダイアログが出るので、VOICEROID.exe を指定します。起動済みの場合、事前に終了させておく必要があります。
    - VOICEROID2 をお持ちの場合、VOICEROID2 のエディタで Ex のデータを使えるようにするアップデータが公式に配布されているので、そちらの利用を強くおすすめいたします。([AHS のマイページ](https://www.ah-soft.com/mypage/)より、製品登録後に入手できます)
  - CeVIO CS: ボタンを押せば自動的に起動します。既に起動している場合は起動中のツールへ接続します。
- 接続後はテキストを入力して "Play" で再生できるようになります。
  - 再生完了後、ファイルへのエクスポートが自動的に行われます ("Export Audio" が有効な場合。デフォルトで有効)
  - ファイルはキャスト、テキスト、パラメータに基づいて生成されます。つまり、同じキャストとテキストでも、パラメータが異なれば別ファイルになります。
  - 一度エクスポートしたテキストは以降ファイルからの再生に切り替わり、ツールがなくても再生できるようになります ("Use Exported Clips" が有効な場合。デフォルトで有効)
- 複数のツールと連動したい場合、RemoteTalkClient も複数用意する必要があります。
  - GameObject が分離している必要はなく、単一の GameObject に複数の RemoteTalkClient を持たせることでも対応可です。
- Timeline 等を使う場合でも、この RemoteTalkClient はどこかに存在している必要があります。

## Timeline
<img align="right" src="https://user-images.githubusercontent.com/1488611/50488723-3df60580-0a47-11e9-8efe-86a52b9816f0.png" width=300>
本プラグインの真価は Timeline との連動にあります。

- Timeline ウィンドウの "Add" から Ist.RemoteTalk -> Remote Talk Track で専用の Track を追加します
- "Add Remote Talk Clip" で Clip を追加、Clip を選択するとキャストやテキストの編集メニューが表示されます。
- テキストファイルから Track や Clip を作成することもできます。長文の場合こちらの方が便利でしょう。
  - Clip 選択時の "Import Text" ボタン、もしくは GameObject -> Remote Talk -> Create Timeline From Text File でインポートします。
  - テキストの内容は [DemoTalk.txt](Assets/Test/Animations/DemoTalks.txt) がいい例になると思われます。
  - 行頭に [キャスト名] があると、以降はそのキャストが発話するテキストになります。
  - {パラメータ名:数値} があると、そのパラメータが適用されます。これは [キャスト名] の行にある場合には以降そのキャストのデフォルトパラメータとして使用され、それ以外の行にある場合はその行のみのパラメータとして扱われます。複数指定する場合 "," で区切ります。例: {高さ:0.8, 抑揚:0.8}
  - インポート時のオプション "Tracks For Each Cast" が有効だと、キャスト毎に Track と AudioSource が生成されます。無効だと単一の Track に全ての Clip が格納されます。
- Clip は複数選択で一括編集が可能です。キャストや読み上げ速度の一括変更などに簡単に対応できます。
- 一度再生した Clip の音声はファイルにエクスポートされ、以降はそのファイルを再生するようになります (RemoteTalkClient の設定に依存。詳細は[基本的な使い方](#基本的な使い方))
  - ファイルエクスポート時には Clip の長さの調節が行われます ("Fit Duration" 有効時。デフォルト有効)。これは、音声の長さは再生してみるまで不明なのが、エクスポートしたことで確定するためです。
  - 同時に、その Clip より後ろにある Clip 郡の開始時間の調整も行われます。例えば、ある Clip が元は 5 秒だったのが 3 秒に調節された場合、その後ろの Clip を 2 秒手前にずらして Clip 間の間隔を保ちます。この挙動は "Arrange Clips" の設定によって変わります。
    - "Current Track" の場合、該当 Clip を含む Track のみ調整が行われます。"All Remote Talk Tracks" の場合、その Timeline 内の全 RemoteTalkTack に対して調整が行われます。"All Tracks" の場合、全種類の Track に対して調整が行われます。None の場合調整は行われません。
    - "All Tracks" が挙動として望ましいことが多いと思われますが、事故の元にもなりそうなので "All Remote Talk Tracks" がデフォルトとなっています。
- ファイルにエクスポートした後は、**"Convert To Audio Track" で AudioTrack に変換が可能です**。全て AudioTrack 化しておけば、ランタイムでは本プラグインは完全に不要になります。さらに、RemoteTalkTrack がサポートしていない blending などのより柔軟な編集ができるようになります。

![timline](https://user-images.githubusercontent.com/1488611/50542180-77bb3d80-0bfa-11e9-8966-c9e54f9c116d.png)

## RemoteTalkScript
<img align="right" src="https://user-images.githubusercontent.com/1488611/50543296-8bc16800-0c17-11e9-8207-665e790b4886.png" width=300>
RemoteTalkScript というコンポーネントでも自動再生が可能です。Timeline がない Unity 5.x でも使用可能で、テキストファイルから Clip の生成にも対応しています。 GameObject -> RemoteTalk -> Create Script で作成できます。  
ただ、2017 以降では Timeline の方がおすすめです。Timeline と比べると、AudioTrack 変換相当機能がない、一括編集できない、他のオブジェクトと連動させるのが大変、などのデメリットがあります。


## その他補足
- 対応するキャストが見つからない Clip は図のようにキャストが Missing 表記になります。これは RemoteTalkClient を用意して対応するツールとつなぎ直せば復旧します。  
![timline](https://user-images.githubusercontent.com/1488611/50543158-a34a2200-0c12-11e9-8910-20ff82f31ae4.png)
  - ファイルにエクスポート済みの音声は RemoteTalkClient の接続が切れていても再生できます。
- RemoteTalkClient の "Sample Granularity" というパラメータは、音の遅延量を示します。小さくすると遅延は少なくなりますが、処理落ちが起きたときなどに音が途切れやすくなります。大きくすると再生が安定しますが、遅延が大きなります。デフォルト値 (8192) はやや安定重視の数値で、VOICEROID2 の場合で 0.2 秒ほど遅延します。


## 注意点
リアルタイムの読み上げには、当然ながら対応ツールがインストールされている必要があります。本プラグインを使用したプロジェクトをビルドして配布する場合、実行する側でもツールが必要になります。  
事前に音声ファイルにエクスポートしていた場合は再生する側でツールは不要になりますが、この場合はライセンス規約に抵触しないように注意が必要です。**VOICEROID も CeVIO も、商用利用には別途有償のライセンス契約が必要になります**。一部例外もあります。詳しくは各製品の商用利用に関するページを参照ください:
- [VOICEROID: 個人向け商用ライセンス](https://www.ah-soft.com/licensee/voice_individual.html)
- [CeVIO: 音声データやキャラクターの利用について](http://cevio.jp/commercial/)
  
また、音声素材のみの公開も制限されています。例えば、エクスポートした音声ファイルを含むプロジェクトを github で公開する、といった行為は避けたほうがいいでしょう。


## License
[MIT](LICENSE.txt)
