function v = INVALID_OBJECT()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = griddynMEX(0, 1);
  end
  v = vInitialized;
end
