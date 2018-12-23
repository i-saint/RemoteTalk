#if UNITY_2017_1_OR_NEWER
using System;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Playables;
using UnityEngine.Timeline;

namespace IST.RemoteTalk
{
    public class RemoteTalkMixerBehaviour : PlayableBehaviour
    {
        public IEnumerable<TimelineClip> clips;
        public PlayableDirector director;
        public RemoteTalkTrack track;

        public override void ProcessFrame(Playable playable, FrameData info, object playerData)
        {
        }
    }
}
#endif
