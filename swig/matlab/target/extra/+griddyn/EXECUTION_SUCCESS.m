function v = EXECUTION_SUCCESS()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = griddynMEX(0, 0);
  end
  v = vInitialized;
end
