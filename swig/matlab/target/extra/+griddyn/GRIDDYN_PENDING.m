function v = GRIDDYN_PENDING()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = griddynMEX(0, 11);
  end
  v = vInitialized;
end
