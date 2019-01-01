#if UNITY_2017_1_OR_NEWER
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    public class RemoteTalkMixerBehaviour : PlayableBehaviour
    {
        public PlayableDirector director;
        public RemoteTalkTrack track;

        public IEnumerable<TimelineClip> clips { get { return track.GetClips(); } }

        public TimelineClip FindClip(int hash)
        {
            return clips.FirstOrDefault(a => ((RemoteTalkClip)a.asset).GetHashCode() == hash);
        }

#if UNITY_EDITOR
        void OnUndoRedo()
        {
            var active = Selection.activeObject;
            if (active != null && active.GetType().FullName == "UnityEditor.Timeline.EditorClip")
            {
                // force refresh timeline window and inspector
                Misc.RefreshTimelineWindow();
                Selection.activeObject = null;
            }
        }

        public override void OnPlayableCreate(Playable playable)
        {
            Undo.undoRedoPerformed += OnUndoRedo;
        }

        public override void OnPlayableDestroy(Playable playable)
        {
            Undo.undoRedoPerformed -= OnUndoRedo;
        }
#endif
    }
}
#endif
