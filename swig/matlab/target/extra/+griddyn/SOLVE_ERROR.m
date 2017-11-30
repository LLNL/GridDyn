function v = SOLVE_ERROR()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = griddynMEX(0, 8);
  end
  v = vInitialized;
end
