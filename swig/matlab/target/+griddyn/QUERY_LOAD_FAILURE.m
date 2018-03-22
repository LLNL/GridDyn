function v = QUERY_LOAD_FAILURE()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = griddynMEX(0, 6);
  end
  v = vInitialized;
end
