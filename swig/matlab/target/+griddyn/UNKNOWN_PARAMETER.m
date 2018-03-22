function v = UNKNOWN_PARAMETER()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = griddynMEX(0, 3);
  end
  v = vInitialized;
end
