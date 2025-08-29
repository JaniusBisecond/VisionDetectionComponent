## 组件功能
检测物品是否在玩家视野范围内，若在视野范围外则随机移动
## 主要内容
EscapeBox——物品随机生成的范围，需要手动添加到Level中  
VisionDetectionComponent——需要检测的物品，添加该组件  
EscapeGameState——管理全局的EscapeBox和VisionDetectionComponent（可以迁移到其他地方，作为GameState只是图方便）
## 使用
1.在Level中添加EscapeBox，可添加多个  
2.需要检测的物品中添加VisionDetectionComponent。运行后，物品会在Level中所有EscapeBox中随机移动  
3.VisionDetectionComponent设置检测模式，默认为EVisionDetectionMode::ClusterMode  
## 检测模式
ClusterMode：所有物品统一由GameState全局管理进行检测。启动：StartClusterVisionDetection(...)  停止：StopClusterVisionDetection()  
IndividualMode：物品单独开启检测，检测时间和间隔不由统一控制。 启动： StartEscape()  停止：StopEscape()  
