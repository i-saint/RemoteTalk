#if UNITY_EDITOR
using UnityEngine;
using UnityEditor;


public class MeshSyncPackaging
{
    [MenuItem("Assets/Make RemoteTalk.unitypackage")]
    public static void MakePackage()
    {
        string[] files = new string[]
        {
            "Assets/IST/RemoteTalk",
            "Assets/StreamingAssets/RemoteTalk",
        };
        AssetDatabase.ExportPackage(files, "RemoteTalk.unitypackage", ExportPackageOptions.Recurse);
    }

}
#endif // UNITY_EDITOR
