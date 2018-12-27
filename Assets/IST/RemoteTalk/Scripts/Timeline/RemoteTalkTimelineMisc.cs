#if UNITY_2017_1_OR_NEWER
using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;
using UnityEngine.SceneManagement;
#if UNITY_EDITOR
using UnityEditor;
using UnityEditor.Timeline;
#endif

namespace IST.RemoteTalk
{
    public static partial class Misc
    {
        public static void EnumerateTracks(TrackAsset track, Action<TrackAsset> act)
        {
            act(track);
            foreach (var child in track.GetChildTracks())
                EnumerateTracks(child, act);
        }

        public static void EnumerateTracks(TimelineAsset timeline, Action<TrackAsset> act)
        {
            foreach (var track in timeline.GetRootTracks())
                EnumerateTracks(track, act);
        }

        public static IEnumerable<TimelineClip> GetRemoteTalkClips(TimelineAsset timeline)
        {
            IEnumerable<TimelineClip> ret = null;
            Misc.EnumerateTracks(timeline, track =>
            {
                var rtt = track as RemoteTalkTrack;
                if (rtt != null)
                {
                    if (ret == null)
                        ret = rtt.GetClips();
                    else
                        ret = ret.Concat(rtt.GetClips());
                }
            });
            if (ret == null)
                ret = new List<TimelineClip>();
            return ret;
        }

        public static void RefreshTimelineWindow()
        {
#if UNITY_EDITOR
#if UNITY_2018_3_OR_NEWER
            TimelineEditor.Refresh(RefreshReason.ContentsAddedOrRemoved);
#else
            // select GO that doesn't have PlayableDirector to clear timeline window
            var roots = SceneManager.GetActiveScene().GetRootGameObjects();
            foreach(var go in roots)
            {
                var director = go.GetComponent<PlayableDirector>();
                if (director == null)
                {
                    Selection.activeGameObject = go;
                    break;
                }
            }
#endif
#endif
        }

    }
}
#endif
