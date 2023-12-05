set(RL_NET_SOURCES
  Native/ErrorCallback.cs
  Native/GCHandleLifetime.cs
  Native/Global.cs
  Native/NativeObject.cs
  Native/SenderAdapter.cs
  Native/StringExtensions.cs

  ActionFlags.cs
  ApiStatus.cs
  AsyncSender.cs
  CALoop.cs
  CBLoop.cs
  CCBLoop.cs
  Configuration.cs
  ContinuousActionResponse.cs
  DecisionResponse.cs
  FactoryContext.cs
  InternalsVisibleToTest.tt
  ILoop.cs
  ISender.cs
  LiveModel.cs
  LiveModelThreadSafe.cs
  MultiSlotResponse.cs
  MultiSlotResponseDetailed.cs
  NativeCallbacks.cs
  RankingResponse.cs
  RLException.cs
  RLLibLogUtils.cs
  SharedBuffer.cs
  SlatesLoop.cs
  SlotRanking.cs
  TraceLogEventArgs.cs
)

if (rlclientlib_DOTNET_USE_MSPROJECT)
  include_external_msproject(rl.net ${CMAKE_CURRENT_SOURCE_DIR}/rl.net.csproj rlnetnative)
else()
  add_custom_target(rl.net
      COMMAND ${DOTNET_COMMAND} build ${CMAKE_CURRENT_SOURCE_DIR} -o $<TARGET_FILE_DIR:rlnetnative> -v n --nologo --no-dependencies /clp:NoSummary --configuration "$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>$<$<CONFIG:RelWithDebInfo>:Release>"
      COMMENT Building rl.net
      DEPENDS rlnetnative
      SOURCES ${RL_NET_SOURCES})
endif()