function v = ADD_FAILURE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = griddynMEX(0, 4);
  end
  v = vInitialized;
end
