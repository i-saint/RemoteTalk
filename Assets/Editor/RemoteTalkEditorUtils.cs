using System.Linq;
using UnityEngine;
using UnityEditor;
using UnityEditorInternal;
using IST.RemoteTalk;

namespace IST.RemoteTalkEditor
{
    public static class RemoteTalkEditorUtils
    {
        public static ReorderableList CreateTalkList(SerializedObject so, SerializedProperty prop)
        {
            var ret = new ReorderableList(so, prop);
            ret.drawHeaderCallback = (rect) =>
            {
                EditorGUI.LabelField(rect, prop.displayName);
            };
            ret.drawElementCallback = (rect, index, isActive, isFocused) =>
            {
                var element = prop.GetArrayElementAtIndex(index);
                rect.height = EditorGUI.GetPropertyHeight(element);
                EditorGUI.PropertyField(rect, element);
            };
            ret.onAddCallback = (list) =>
            {
                prop.arraySize++;
                list.index = prop.arraySize - 1;
            };
            ret.elementHeightCallback = (int i) =>
            {
                var element = prop.GetArrayElementAtIndex(i);
                return EditorGUI.GetPropertyHeight(element);
            };
            return ret;
        }

        [MenuItem("GameObject/Remote Talk/Create Client", false, 10)]
        public static void CreateRemoteTalkClient(MenuCommand menuCommand)
        {
            var audio = new GameObject();
            audio.name = "RemoteTalkAudio";
            var output = audio.AddComponent<AudioSource>();
            audio.AddComponent<RemoteTalkAudio>();

            var client = new GameObject();
            client.name = "RemoteTalkClient";
            var rtc = client.AddComponent<RemoteTalkClient>();
            rtc.output = output;

            Selection.activeGameObject = client;

            Undo.RegisterCreatedObjectUndo(audio, "RemoteTalk");
            Undo.RegisterCreatedObjectUndo(client, "RemoteTalk");
        }

        [MenuItem("GameObject/Remote Talk/Create Script", false, 10)]
        public static void CreateRemoteTalkScript(MenuCommand menuCommand)
        {
            var go = new GameObject();
            go.name = "RemoteTalkScript";
            go.AddComponent<RemoteTalkScript>();

            Undo.RegisterCreatedObjectUndo(go, "RemoteTalk");
        }

        //[MenuItem("Debug/Remote Talk/List All Casts", false, 10)]
        //public static void ListAllCasts(MenuCommand menuCommand)
        //{
        //    string result = "";
        //    foreach (var c in RemoteTalkProvider.allCasts)
        //    {
        //        result += c.name + "\n";
        //        result += "  host: " + c.hostName + "\n";
        //        result += "  params:\n";
        //        foreach (var pi in c.paramInfo)
        //            result += "    " + pi.name + "\n";
        //        result += "\n";
        //    }
        //    Debug.Log(result);
        //}
    }
}
