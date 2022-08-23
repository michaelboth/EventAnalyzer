Test examples:
  Mac & Linux:
    > cd packaging
    > ./buildExamples.bash
    Use visualizer to view event files
    > ./buildExamples.bash clean

  Windows:
    > cd packaging
    > buildExamples
    Use visualizer to view event files
    > buildExamples clean


Build release packages:
  Linux:
    > cd packaging
    > ./buildLinuxPackage.bash $HOME/Qt/5.15.2 1.0.0
  Mac
    > cd packaging
    > ./buildMacPackage.bash $HOME/Qt/5.15.2 1.0.0
  Windows
    > cd packaging
    > buildWindowsPackage C:\Qt\5.15.2 1.0.0
