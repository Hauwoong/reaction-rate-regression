% visualize.m
% H2O2 분해 속도 예측 - 회귀 결과 시각화
%
% 사용법
%   1) C++ 프로그램에서 [7] CSV 내보내기 -> [5] 전체 내보내기 를 실행
%   2) 아래 stem 을 그때 입력한 파일명으로 바꾼다
%   3) MATLAB에서 이 스크립트를 실행

clear; close all; clc;

% ─────────────────────────────────────────────
%  설정
% ─────────────────────────────────────────────
stem   = 'exp_0911';   % [7] 에서 입력한 파일명
outdir = 'output';          % CSV 가 있는 폴더
savePng = true;             % 그림을 PNG로도 저장할지

nSlices = 3;                % Figure 1 에서 겹쳐 그릴 H2O2 농도 개수

% ─────────────────────────────────────────────
%  CSV 읽기
% ─────────────────────────────────────────────
fData    = fullfile(outdir, [stem '_data.csv']);
fReg     = fullfile(outdir, [stem '_regression.csv']);
fPred    = fullfile(outdir, [stem '_prediction.csv']);
fSurface = fullfile(outdir, [stem '_surface.csv']);

for f = {fReg, fPred, fSurface}
    if ~isfile(f{1})
        error('파일을 찾을 수 없습니다: %s\n[7] 메뉴에서 전체 내보내기를 먼저 실행하세요.', f{1});
    end
end

R = readtable(fReg);
P = readtable(fPred);
S = readtable(fSurface);

% 회귀 지표 꺼내기
r2   = R.value(strcmp(R.variable, 'r_squared'));
rmse = R.value(strcmp(R.variable, 'rmse'));
n    = R.value(strcmp(R.variable, 'n_samples'));

fprintf('데이터 %d건 | R^2 = %.4f | RMSE = %.4f\n', n, r2, rmse);

% ─────────────────────────────────────────────
%  Figure 1 : 3D 표면 (온도 x 촉매 -> 예측 속도)
%             H2O2 농도별로 여러 장을 겹쳐 그린다
% ─────────────────────────────────────────────
figure('Name', 'Figure 1 - 예측 표면', 'Color', 'w');
hold on;

hv = unique(S.h2o2_M);
pick = round(linspace(1, numel(hv), nSlices));   % 최소 ~ 최대에서 골고루

colors = lines(nSlices);
legendText = strings(1, nSlices);

for idx = 1:nSlices
    h = hv(pick(idx));

    % 이 농도의 격자만 뽑아서 (온도, 촉매) 순으로 정렬
    slice = S(abs(S.h2o2_M - h) < 1e-9, :);
    slice = sortrows(slice, {'temperature_C', 'catalyst_g'});

    tv = unique(slice.temperature_C);
    cv = unique(slice.catalyst_g);

    % 행 순서가 온도 바깥 / 촉매 안쪽이므로 [촉매 x 온도] 로 접는다
    Z = reshape(slice.predicted, numel(cv), numel(tv));

    surf(tv, cv, Z, 'FaceAlpha', 0.65, 'EdgeColor', 'none', ...
         'FaceColor', colors(idx, :));

    legendText(idx) = sprintf('H_2O_2 = %.2f M', h);
end

xlabel('온도 (°C)');
ylabel('촉매 질량 (g)');
zlabel('예측 산소 발생 속도 (mL/s)');
title('Figure 1 — 회귀 모델의 예측 표면');
legend(legendText, 'Location', 'northwest');
view(-40, 25);
grid on;
colorbar off;
hold off;

% ─────────────────────────────────────────────
%  Figure 2 : 실측 vs 예측 산점도
%             점이 y=x 위에 몰릴수록 모델이 정확하다
% ─────────────────────────────────────────────
figure('Name', 'Figure 2 - 실측 vs 예측', 'Color', 'w');

scatter(P.o2_rate_mL_s, P.predicted, 70, 'filled', ...
        'MarkerFaceAlpha', 0.75);
hold on;

lim = [min([P.o2_rate_mL_s; P.predicted]), max([P.o2_rate_mL_s; P.predicted])];
pad = 0.05 * diff(lim);
lim = [lim(1) - pad, lim(2) + pad];

plot(lim, lim, 'r--', 'LineWidth', 1.2);      % y = x 기준선

xlim(lim); ylim(lim);
axis square;
xlabel('실측 속도 (mL/s)');
ylabel('예측 속도 (mL/s)');
title(sprintf('Figure 2 — 실측 vs 예측  (R^2 = %.4f, RMSE = %.4f)', r2, rmse));
legend({'실험 데이터', 'y = x (완벽한 예측)'}, 'Location', 'northwest');
grid on;
hold off;

% ─────────────────────────────────────────────
%  Figure 3 : 변인 영향도
%             |β| 를 그냥 비교하면 안 되므로
%             C++ 이 계산해 둔 |β| x (실험 범위) 를 쓴다
% ─────────────────────────────────────────────
figure('Name', 'Figure 3 - 변인 영향도', 'Color', 'w');

isBeta = startsWith(R.term, 'beta') & ~strcmp(R.variable, 'intercept');

names  = R.variable(isBeta);
betas  = abs(R.value(isBeta));
infl   = R.influence(isBeta);

labels = {'온도', '촉매 질량', 'H_2O_2 농도'};

subplot(1, 2, 1);
bar(betas, 'FaceColor', [0.7 0.7 0.7]);
set(gca, 'XTickLabel', labels);
ylabel('|\beta|');
title({'계수 절댓값', '(단위가 달라 직접 비교 불가)'});
grid on;

subplot(1, 2, 2);
bar(infl, 'FaceColor', [0.20 0.45 0.70]);
set(gca, 'XTickLabel', labels);
ylabel('영향도 (mL/s)');
title({'|\beta| \times 실험 범위', '(공정한 비교)'});
grid on;

sgtitle('Figure 3 — 변인별 영향도');

[~, top] = max(infl);
fprintf('실험 범위 기준 영향이 가장 큰 변인: %s (%.4f mL/s)\n', names{top}, infl(top));

% ─────────────────────────────────────────────
%  Figure 4 : 잔차 분석
%             잔차가 0 주변에 무작위로 흩어져야 선형 모델이 적절하다
% ─────────────────────────────────────────────
figure('Name', 'Figure 4 - 잔차 분석', 'Color', 'w');

subplot(1, 2, 1);
histogram(P.residual, max(5, round(sqrt(height(P)))), ...
          'FaceColor', [0.20 0.45 0.70]);
xline(0, 'r--', 'LineWidth', 1.2);
xlabel('잔차 (실측 - 예측)');
ylabel('빈도');
title('잔차 분포');
grid on;

subplot(1, 2, 2);
scatter(P.predicted, P.residual, 70, 'filled', 'MarkerFaceAlpha', 0.75);
yline(0, 'r--', 'LineWidth', 1.2);
xlabel('예측 속도 (mL/s)');
ylabel('잔차 (mL/s)');
title({'예측값 대 잔차', '(휘어 있으면 곡선 관계를 놓친 것)'});
grid on;

sgtitle('Figure 4 — 잔차 분석');

% ─────────────────────────────────────────────
%  PNG 저장
% ─────────────────────────────────────────────
if savePng
    figs = findobj('Type', 'figure');
    figs = flipud(figs);      % 생성 순서대로

    for i = 1:numel(figs)
        fname = fullfile(outdir, sprintf('%s_figure%d.png', stem, i));
        exportgraphics(figs(i), fname, 'Resolution', 150);
        fprintf('저장: %s\n', fname);
    end
end
