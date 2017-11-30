function v = INVALID_PARAMETER_VALUE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = griddynMEX(0, 2);
  end
  v = vInitialized;
end
