function v = FILE_LOAD_FAILURE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = griddynMEX(0, 7);
  end
  v = vInitialized;
end
