using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    [Serializable]
    public class TalkParam
    {
        public string name;
        public float value, rangeMin, rangeMax;
    }

    [Serializable]
    public class Cast
    {
        public string hostName;
        public int id;
        public string name;
        public TalkParam[] paramInfo;

        public RemoteTalkProvider provider
        {
            get { return RemoteTalkProvider.FindByHostName(hostName); }
        }
    }

    [Serializable]
    public class Talk
    {
        public string castName = "";
        public TalkParam[] param;
        public string text = "";
#if UNITY_EDITOR
        public bool foldParams;
#endif

        public Cast cast
        {
            get { return RemoteTalkProvider.FindCast(castName); }
        }
        public RemoteTalkProvider provider
        {
            get { return RemoteTalkProvider.FindByCastName(castName); }
        }
        public AudioClip audioClip
        {
            get
            {
                var prov = provider;
                if (prov != null)
                    return prov.FindClip(this);
                else
                {
                    foreach (var p in RemoteTalkProvider.instances)
                    {
                        var clip = p.FindClip(this);
                        if (clip != null)
                            return clip;
                    }
                }
                return null;
            }
        }

        public bool Play()
        {
            //cast?.provider?.Talk(this);
            var c = cast;
            if (c != null)
            {
                var prov = c.provider;
                if (prov != null)
                    return prov.Play(this);
            }
            return false;
        }

        public void Stop()
        {
            var c = cast;
            if (c != null)
            {
                var prov = c.provider;
                if (prov != null)
                    prov.Stop();
            }
        }
    }

    public delegate void AudioClipImportCallback(Talk talk, AudioClip clip);


    public abstract class RemoteTalkProvider : MonoBehaviour
    {
        static List<RemoteTalkProvider> s_instances = new List<RemoteTalkProvider>();

        public static List<RemoteTalkProvider> instances { get { return s_instances; } }

        public static event AudioClipImportCallback onAudioClipImport;

        public static IEnumerable<Cast> allCasts
        {
            get
            {
                var providers = instances;
                if (providers.Count == 0)
                    return new Cast[0] { };
                IEnumerable<Cast> ret = providers[0].casts;
                for (int i = 1; i < providers.Count; ++i)
                    ret = ret.Concat(providers[i].casts);
                return ret;
            }
        }

        public static Cast FindCast(string castName)
        {
            return allCasts.FirstOrDefault(a => a.name == castName);
        }

        public static RemoteTalkProvider FindByHostName(string hostName)
        {
            return instances.FirstOrDefault(p => p.hostName == hostName);
        }

        public static RemoteTalkProvider FindByCastName(string castName)
        {
            return instances.FirstOrDefault(p =>
            {
                return p.casts.FirstOrDefault(c => c.name == castName) != null;
            });
        }

        public static void FireOnAudioClipImport(Talk t, AudioClip ac)
        {
            if (onAudioClipImport != null)
                onAudioClipImport(t, ac);
        }


        public abstract string hostName { get; }
        public abstract Cast[] casts { get; }
        public abstract bool isPlaying { get; }
        public abstract bool isReady { get; }
        public abstract AudioSource output { get; set; }

        public abstract bool Play(Talk talk);
        public abstract void Stop();

        public abstract AudioClip FindClip(Talk talk);


        protected virtual void OnEnable()
        {
            instances.Add(this);
        }

        protected virtual void OnDisable()
        {
            instances.Remove(this);
        }
    }
}
