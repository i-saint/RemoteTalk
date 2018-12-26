#if UNITY_EDITOR
#if UNITY_2017_1_OR_NEWER
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;
using UnityEditor;

namespace IST.RemoteTalk
{
    public class ImportTextWindow : EditorWindow
    {
        public string path;
        public bool parCastTrack = true;

        [MenuItem("Assets/RemoteTalk/Import Text")]
        static void Init()
        {
            var path = EditorUtility.OpenFilePanel("Import Text", ".", "txt");
            if (path.Length != 0)
            {
                var window = (ImportTextWindow)EditorWindow.GetWindow(typeof(ImportTextWindow));
                window.titleContent = new GUIContent("Import Text");
                window.path = path;
                window.Show();

            }
        }

        void OnGUI()
        {
            EditorGUI.BeginDisabledGroup(true);
            GUILayout.TextField(path);
            EditorGUI.EndDisabledGroup();
            parCastTrack =  EditorGUILayout.Toggle("Tracks for each cast", parCastTrack);

            if (GUILayout.Button("Create Timeline Track"))
            {
                var talks = RemoteTalkScript.TextFileToTalks(path);

                TimelineAsset timeline = null;
                PlayableDirector director = null;
                var go = Selection.activeGameObject;
                if (go != null)
                {
                    director = go.GetComponent<PlayableDirector>();
                    if (director != null)
                        timeline = director.playableAsset as TimelineAsset;
                }

                if (timeline == null)
                {
                    timeline = ScriptableObject.CreateInstance<TimelineAsset>();
                    AssetDatabase.CreateAsset(timeline, "Assets/RemoteTalkTimeline.asset");
                    go = new GameObject();
                    go.name = "RemoteTalkTimeline";
                    director = go.AddComponent<PlayableDirector>();
                    director.playableAsset = timeline;
                }
                CreateTracks(director, timeline, talks);
                RemoteTalkTrack.RefreshTimelineWindow();

                Close();
            }
        }

        TimelineAsset CreateTracks(PlayableDirector director, TimelineAsset timeline, IEnumerable<Talk> talks)
        {
            double time = 0.0;
            if (parCastTrack)
            {
                var tracks = new Dictionary<string, RemoteTalkTrack>();
                foreach (var talk in talks)
                {
                    RemoteTalkTrack track = null;
                    if (!tracks.TryGetValue(talk.castName, out track))
                    {
                        track = timeline.CreateTrack<RemoteTalkTrack>(null, "RemoteTalk");
                        track.director = director;
                        track.name = talk.castName;
                        tracks[talk.castName] = track;

                        var audio = new GameObject();
                        audio.name = talk.castName;
                        track.audioSource = audio.AddComponent<AudioSource>();
                    }
                    var clip = track.AddClip(talk);
                    clip.start = time;
                    time += clip.duration;
                }
            }
            else
            {
                var track = timeline.CreateTrack<RemoteTalkTrack>(null, "RemoteTalk");
                track.director = director;
                track.name = "RemoteTalkTrack";

                var audio = new GameObject();
                audio.name = "RemoteTalkAudioSource";
                track.audioSource = audio.AddComponent<AudioSource>();

                foreach (var talk in talks)
                {
                    var clip = track.AddClip(talk);
                    clip.start = time;
                    time += clip.duration;
                }
            }
            return timeline;
        }
    }
}
#endif
#endif
