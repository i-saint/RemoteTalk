#if UNITY_2017_1_OR_NEWER
using UnityEngine;
using UnityEngine.Playables;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    public class RemoteTalkMixerBehaviour : PlayableBehaviour
    {
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
