function v = INVALID_FUNCTION_CALL()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = griddynMEX(0, 10);
  end
  v = vInitialized;
end
