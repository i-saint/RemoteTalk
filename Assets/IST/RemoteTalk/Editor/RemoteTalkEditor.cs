#if UNITY_EDITOR
using System.Linq;
using UnityEngine;
using UnityEditor;

namespace IST.RemoteTalk
{
    public static class RemoteTalkEditor
    {
        public static void DrawTalkParams(int castID, SerializedProperty talkParamsProp, RemoteTalkClient client)
        {
            var casts = client.casts;
            if (casts.Length == 0)
                return;

            var castNames = casts.Select(a => a.name).ToArray();

            var castProp = talkParamsProp.FindPropertyRelative("cast");
            castProp.intValue = EditorGUILayout.Popup("Cast", castProp.intValue, castNames);

            if (castID >= 0 && castID < casts.Length)
            {
                var castInfo = casts[castID];
                var paramValues = talkParamsProp.FindPropertyRelative("paramValues");
                for (int pi = 0; pi < castInfo.paramInfo.Length; ++pi)
                {
                    var p = paramValues.GetFixedBufferElementAtIndex(pi);
                    EditorGUILayout.PropertyField(p, new GUIContent(castInfo.paramInfo[pi].name));
                }
            }
        }
    }
}
#endif
