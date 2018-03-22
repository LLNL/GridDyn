function v = OBJECT_NOT_INITIALIZED()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = griddynMEX(0, 9);
  end
  v = vInitialized;
end
