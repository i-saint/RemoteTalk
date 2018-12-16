using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace IST.RemoteTalk
{
    public abstract class RemoteTalkProvider : MonoBehaviour
    {
        static List<RemoteTalkProvider> s_instances = new List<RemoteTalkProvider>();

        public static List<RemoteTalkProvider> instances { get { return s_instances; } }

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

        public static Cast FindCast(string name)
        {
            foreach (var c in allCasts)
                if (c.name == name)
                    return c;
            return null;
        }

        public static RemoteTalkProvider FindByHost(string host)
        {
            foreach (var c in instances)
            {
                if (c.host == host)
                    return c;
            }
            return null;
        }

        public static RemoteTalkProvider FindByCast(string cast)
        {
            foreach (var p in instances)
                foreach (var c in p.casts)
                    if (c.name == cast)
                        return p;
            return null;
        }


        public abstract string host { get; }
        public abstract Cast[] casts { get; }


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