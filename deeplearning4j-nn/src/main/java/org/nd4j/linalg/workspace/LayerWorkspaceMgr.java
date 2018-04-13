package org.nd4j.linalg.workspace;

import com.google.common.base.Preconditions;
import org.nd4j.linalg.api.memory.conf.WorkspaceConfiguration;
import org.nd4j.linalg.api.ndarray.INDArray;

import java.lang.reflect.Array;
import java.util.Collections;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class LayerWorkspaceMgr extends BaseWorkspaceMgr<ArrayType> {

    private static LayerWorkspaceMgr NO_WS_IMMUTABLE;
    static{
        Set<ArrayType> all = new HashSet<>();
        Collections.addAll(all, ArrayType.values());
        NO_WS_IMMUTABLE = new LayerWorkspaceMgr(
                all, Collections.<ArrayType, WorkspaceConfiguration>emptyMap(), Collections.<ArrayType, String>emptyMap());
    }
    
    private LayerWorkspaceMgr(){

    }

    public LayerWorkspaceMgr(Set<ArrayType> scopeOutOfWs, Map<ArrayType, WorkspaceConfiguration> configMap,
                             Map<ArrayType, String> workspaceNames){
        super(scopeOutOfWs, configMap, workspaceNames);
        if(configMap != null){
            Preconditions.checkArgument(configMap.keySet().equals(workspaceNames.keySet()),
                    "Keys for config may and workspace names must match");
        }
    }

    public static Builder builder(){
        return new Builder();
    }

    public static LayerWorkspaceMgr noWorkspaces(){
        return builder().defaultNoWorkspace().build();
    }

    public static LayerWorkspaceMgr noWorkspacesImmutable(){
        return NO_WS_IMMUTABLE;
    }

    public static class Builder {

        private LayerWorkspaceMgr mgr;

        public Builder(){
            mgr = new LayerWorkspaceMgr();
        }

        public Builder defaultNoWorkspace(){
            for(ArrayType t : ArrayType.values()){
                if(!mgr.configMap.containsKey(t)){
                    mgr.setScopedOutFor(t);
                }
            }
            return this;
        }

        public Builder noWorkspaceFor(ArrayType type){
            mgr.setScopedOutFor(type);
            return this;
        }

        public Builder defaultWorkspace(String workspaceName, WorkspaceConfiguration configuration){
            for(ArrayType t : ArrayType.values()){
                if(!mgr.configMap.containsKey(t)){
                    with(t, workspaceName, configuration);
                }
            }
            return this;
        }

        public Builder with(ArrayType type, String workspaceName, WorkspaceConfiguration configuration){
            mgr.setConfiguration(type, configuration);
            mgr.setWorkspaceName(type, workspaceName);
            return this;
        }

        public LayerWorkspaceMgr build(){
            return mgr;
        }

    }
    
}
