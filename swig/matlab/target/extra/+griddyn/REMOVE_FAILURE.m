function v = REMOVE_FAILURE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = griddynMEX(0, 5);
  end
  v = vInitialized;
end
